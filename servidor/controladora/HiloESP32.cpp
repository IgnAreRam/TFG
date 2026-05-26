#include "HiloESP32.h"
#include "Esp32.h"
#include "Esp32DAO.h"
#include "RegistroIA.h"
#include "RegistroIADAO.h"
#include "Medio.h"
#include "MedioDAO.h"
#include "ServicioIA.h"
#include "GestorConexiones.h"
#include "ConexionBD.h"
#include "Protocolo.h"
#include "Logger.h"
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <sys/stat.h>
#include <sys/socket.h>

using namespace std;

// La ESP32-CAM envía a 320x240 (QVGA); por eso la IA centra respecto a ese tamaño.
static const int ANCHO = 320;
static const int ALTO  = 240;
static const int ZONA     = 10;   // zona muerta central (px): dentro no se mueve
static const int PASO_MAX = 12;   // grados máximos por ajuste (limita el control proporcional)
// Cámara en landscape (girada 90°): cx controla el TILT y cy controla el PAN.
// Si un eje persigue al revés (aleja el objeto), cambia su SENTIDO a -1.
static const int SENTIDO_PAN  = 1;
static const int SENTIDO_TILT = 1;
static const int SEG_CLIP_IA = 8;   // segundos sin detección para cerrar el clip de la IA

// Contador global para que los nombres de archivo de medios no se repitan.
static atomic<long> contadorMedios{0};
static string selloUnico() {
    return to_string(time(nullptr)) + "_" + to_string(contadorMedios++);
}

// Datos (copiados por valor) para codificar un clip en un hilo aparte, sin tocar el objeto HiloESP32.
struct DatosCodificacion {
    ConexionBD* bd;     // vive todo el programa (creado en main) → seguro de usar aquí
    int idEsp32;
    int usuario;
    string origen;      // "manual" | "ia"
    string dir;         // carpeta temporal con los f_*.jpg
    string salida;      // ruta del .mp4 final
};

// ── HILO: codifica el clip con ffmpeg y lo registra en la BD, SIN bloquear al hilo de IA ──
static void* codificarVideo(void* arg) {
    DatosCodificacion* d = (DatosCodificacion*)arg;
    // transpose=1 gira el vídeo 90° (la cámara va en landscape), 10 fps de reproducción.
    string cmd = "ffmpeg -y -framerate 10 -i " + d->dir +
                 "/f_%05d.jpg -vf transpose=1 -c:v libx264 -pix_fmt yuv420p " + d->salida + " >/dev/null 2>&1";
    system(cmd.c_str());
    system(("rm -rf " + d->dir).c_str());   // borrar los frames temporales
    Medio m(d->idEsp32, d->usuario, "video", d->origen, d->salida);
    MedioDAO::insertar(d->bd, m);            // se inserta cuando el mp4 YA existe
    Logger::ok("Vídeo guardado: " + d->salida);
    delete d;                                // liberar los datos del hilo
    return nullptr;
}

// ╔══════════════════════════════════════╗
// ║   CONSTRUCTOR / DESTRUCTOR           ║
// ╚══════════════════════════════════════╝
HiloESP32::HiloESP32(int socketFd, ConexionBD* bd, GestorConexiones* gestor,
                     ServicioIA* ia, const string& dirMedios) {
    this->socketFd = socketFd;
    this->bd = bd;
    this->gestor = gestor;
    this->ia = ia;
    this->dirMedios = dirMedios;
    esp32 = nullptr;
    conectado = true;
    ultimoFrame = nullptr;
    tamUltimoFrame = 0;
    contadorFrame = 0;
    panActual = 90;    // servos centrados al empezar
    tiltActual = 90;
    ultimoServoManual = 0;
    grabando = false;
    framesGrabados = 0;
    usuarioGrabacion = -1;
    ultimaDeteccionIA = 0;
    pthread_mutex_init(&mutexFrame, nullptr);
    pthread_cond_init(&condFrame, nullptr);
    pthread_mutex_init(&mutexEnvio, nullptr);
    pthread_mutex_init(&mutexGrabacion, nullptr);
}

HiloESP32::~HiloESP32() {
    delete[] ultimoFrame;   // libera el buffer del último frame JPEG
    delete esp32;           // libera los datos de la cámara autenticada
    if (socketFd >= 0) close(socketFd);
    pthread_mutex_destroy(&mutexFrame);
    pthread_cond_destroy(&condFrame);
    pthread_mutex_destroy(&mutexEnvio);
    pthread_mutex_destroy(&mutexGrabacion);
}

// ╔══════════════════════════════════════╗
// ║   BUCLE PRINCIPAL (HILO LECTOR)      ║
// ╚══════════════════════════════════════╝
void HiloESP32::ejecutar() {
    if (!autenticar()) return;

    gestor->registrar(this);
    Logger::ok("Cámara conectada: " + esp32->getNombre());

    // Flash de bienvenida: LED on, esperar 400ms, LED off.
    enviarLed(true);
    usleep(400000);
    enviarLed(false);

    // ── HILO: lanzar el hilo de IA para que YOLO no bloquee la lectura del socket ──
    pthread_create(&hiloIA, nullptr, funcionIA, this);

    // Bucle lector: recibe frames hasta que la cámara se desconecta.
    uint8_t tipo;
    unsigned char* payload;
    uint32_t longitud;
    while (leerMensaje(socketFd, tipo, payload, longitud)) {
        if (tipo == MSG_FRAME) guardarFrame(payload, longitud);
        delete[] payload;   // libera el payload reservado por leerMensaje
    }

    // --- Desconexión: avisar al hilo IA y esperar a que termine ---
    conectado = false;
    // ── SINCRONIZACIÓN: despertar al hilo IA por si está esperando un frame ──
    pthread_mutex_lock(&mutexFrame);
    pthread_cond_broadcast(&condFrame);
    pthread_mutex_unlock(&mutexFrame);

    gestor->eliminar(this);
    // ── HILO: esperar a que el hilo IA termine antes de destruir este objeto ──
    pthread_join(hiloIA, nullptr);
    pararGrabacion();   // si había un clip a medias, lo cerramos antes de salir
    Logger::warning("Cámara desconectada: " + esp32->getNombre());
}

// ╔══════════════════════════════════════╗
// ║   AUTENTICACIÓN                      ║
// ╚══════════════════════════════════════╝
bool HiloESP32::autenticar() {
    uint8_t tipo;
    unsigned char* payload;
    uint32_t longitud;
    if (!leerMensaje(socketFd, tipo, payload, longitud)) return false;

    if (tipo != MSG_ESP32_AUTH) {
        delete[] payload;
        enviarMensaje(socketFd, MSG_AUTH_FAIL, nullptr, 0);
        return false;
    }

    // payload = "nombre:clave"
    string texto((char*)payload, longitud);
    delete[] payload;
    size_t sep = texto.find(':');
    string nombre = texto.substr(0, sep);
    string clave  = texto.substr(sep + 1);

    esp32 = Esp32DAO::autenticar(bd, nombre, clave);
    if (esp32) {
        enviarMensaje(socketFd, MSG_AUTH_OK, nullptr, 0);
        return true;
    }
    enviarMensaje(socketFd, MSG_AUTH_FAIL, nullptr, 0);
    Logger::warning("Auth fallida de cámara: " + nombre);
    return false;
}

// ╔══════════════════════════════════════╗
// ║   BUFFER DE FRAME COMPARTIDO         ║
// ╚══════════════════════════════════════╝
void HiloESP32::guardarFrame(const unsigned char* datos, int tam) {
    // ── SINCRONIZACIÓN: escribir el frame compartido bajo mutex ──
    pthread_mutex_lock(&mutexFrame);
    delete[] ultimoFrame;                       // soltar el frame anterior
    ultimoFrame = new unsigned char[tam];        // reservar para el nuevo
    memcpy(ultimoFrame, datos, tam);
    tamUltimoFrame = tam;
    contadorFrame++;                             // marca que hay un frame nuevo sin procesar
    // ── SINCRONIZACIÓN: avisar a la IA (y a los Flutter) de que hay frame nuevo ──
    pthread_cond_broadcast(&condFrame);
    pthread_mutex_unlock(&mutexFrame);

    // Si hay una grabación en curso, guardamos también este frame en disco
    // (fuera del mutexFrame para no bloquear a la IA ni a Flutter con la escritura).
    pthread_mutex_lock(&mutexGrabacion);
    if (grabando) {
        char nombre[300];
        snprintf(nombre, sizeof(nombre), "%s/f_%05d.jpg", dirGrabacion.c_str(), framesGrabados++);
        FILE* f = fopen(nombre, "wb");
        if (f) { fwrite(datos, 1, tam, f); fclose(f); }
    }
    pthread_mutex_unlock(&mutexGrabacion);
}

int HiloESP32::copiarUltimoFrame(unsigned char* destino, int maxTam) {
    int n = 0;
    pthread_mutex_lock(&mutexFrame);
    if (ultimoFrame && tamUltimoFrame > 0 && tamUltimoFrame <= maxTam) {
        memcpy(destino, ultimoFrame, tamUltimoFrame);
        n = tamUltimoFrame;
    }
    pthread_mutex_unlock(&mutexFrame);
    return n;
}

long HiloESP32::getContadorFrame() {
    pthread_mutex_lock(&mutexFrame);
    long c = contadorFrame;
    pthread_mutex_unlock(&mutexFrame);
    return c;
}

// ╔══════════════════════════════════════╗
// ║   ENVÍOS AL ESP32 (servo / led)      ║
// ╚══════════════════════════════════════╝
void HiloESP32::enviarServo(int pan, int tilt) {
    unsigned char p[4] = {
        (unsigned char)((pan >> 8) & 0xFF),  (unsigned char)(pan & 0xFF),
        (unsigned char)((tilt >> 8) & 0xFF), (unsigned char)(tilt & 0xFF) };
    // ── SINCRONIZACIÓN: IA y Flutter pueden escribir a la vez; serializamos con mutex ──
    pthread_mutex_lock(&mutexEnvio);
    enviarMensaje(socketFd, MSG_SERVO_CMD, p, 4);
    pthread_mutex_unlock(&mutexEnvio);
}

void HiloESP32::enviarLed(bool encendido) {
    unsigned char b = encendido ? 1 : 0;
    pthread_mutex_lock(&mutexEnvio);
    enviarMensaje(socketFd, MSG_LED_CMD, &b, 1);
    pthread_mutex_unlock(&mutexEnvio);
}

void HiloESP32::marcarServoManual() {
    // ── ATÓMICO: sin mutex porque atomic<time_t> garantiza visibilidad entre hilos ──
    ultimoServoManual.store(time(nullptr));
}

// ╔══════════════════════════════════════╗
// ║   HILO DE IA                         ║
// ╚══════════════════════════════════════╝
void* HiloESP32::funcionIA(void* arg) {
    ((HiloESP32*)arg)->bucleIA();
    return nullptr;
}

void HiloESP32::bucleIA() {
    long procesado = 0;
    while (conectado) {
        // ── SINCRONIZACIÓN: esperar a que llegue un frame nuevo ──
        pthread_mutex_lock(&mutexFrame);
        while (contadorFrame == procesado && conectado) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 2;   // máx 2s: así re-comprobamos 'conectado' si la cámara se va
            pthread_cond_timedwait(&condFrame, &mutexFrame, &ts);
        }
        if (!conectado) { pthread_mutex_unlock(&mutexFrame); break; }

        // Copiamos el frame para soltar el mutex cuanto antes (YOLO tarda).
        int tam = tamUltimoFrame;
        unsigned char* copia = new unsigned char[tam];
        memcpy(copia, ultimoFrame, tam);
        procesado = contadorFrame;
        pthread_mutex_unlock(&mutexFrame);

        // ── ATÓMICO: si Flutter movió el servo hace <8s, la IA se aparta ──
        if (time(nullptr) - ultimoServoManual.load() < 8) { delete[] copia; continue; }

        Deteccion d = ia->analizar(copia, tam);
        delete[] copia;   // ya no necesitamos esta copia del frame

        if (d.detectado) {
            ajustarServos(d.cx, d.cy);
            RegistroIA r(esp32->getId(), d.tipo, d.confianza);
            RegistroIADAO::insertar(bd, r);
            Logger::ok("IA [" + esp32->getNombre() + "] detectó " + d.tipo);
            ultimaDeteccionIA = time(nullptr);
            iniciarGrabacion("ia", -1);   // arranca un clip si no había grabación en curso
        }

        // Cierra el clip de la IA si han pasado varios segundos sin detección.
        if (estaGrabandoIA() && time(nullptr) - ultimaDeteccionIA > SEG_CLIP_IA)
            pararGrabacion();
    }
}

// Mueve los servos un paso para acercar el objeto al centro de la imagen.
// El signo depende de cómo esté montado físicamente el servo (puede invertirse).
void HiloESP32::ajustarServos(int cx, int cy) {
    // Landscape (cámara girada 90°): cy mueve el PAN, cx mueve el TILT.
    int errorPan  = cy - ALTO  / 2;
    int errorTilt = cx - ANCHO / 2;
    bool mover = false;

    // Control proporcional: el paso crece con el error (lejos = paso grande, cerca = pequeño),
    // limitado a PASO_MAX. Así centra suave y en pocos pasos en vez de a tirones fijos.
    if (errorPan > ZONA || errorPan < -ZONA) {
        int paso = errorPan / 8;
        if (paso > PASO_MAX)  paso = PASO_MAX;
        if (paso < -PASO_MAX) paso = -PASO_MAX;
        panActual += SENTIDO_PAN * paso;
        mover = true;
    }
    if (errorTilt > ZONA || errorTilt < -ZONA) {
        int paso = errorTilt / 8;
        if (paso > PASO_MAX)  paso = PASO_MAX;
        if (paso < -PASO_MAX) paso = -PASO_MAX;
        tiltActual += SENTIDO_TILT * paso;
        mover = true;
    }

    if (panActual < 0) panActual = 0;   if (panActual > 180) panActual = 180;
    if (tiltActual < 0) tiltActual = 0; if (tiltActual > 180) tiltActual = 180;

    Logger::debug("IA seguimiento cx=" + to_string(cx) + " cy=" + to_string(cy) +
                  " errPan=" + to_string(errorPan) + " errTilt=" + to_string(errorTilt) +
                  (mover ? (" -> MUEVE pan=" + to_string(panActual) + " tilt=" + to_string(tiltActual))
                         : " -> centrado, no mueve"));

    if (mover) enviarServo(panActual, tiltActual);
}

// ╔══════════════════════════════════════╗
// ║   FOTO Y GRABACIÓN DE VÍDEO          ║
// ╚══════════════════════════════════════╝
void HiloESP32::capturarFoto(int idUsuario) {
    unsigned char* buffer = new unsigned char[MAX_PAYLOAD];
    int tam = copiarUltimoFrame(buffer, MAX_PAYLOAD);
    if (tam > 0) {
        string ruta = dirMedios + "/foto_" + selloUnico() + ".jpg";
        FILE* f = fopen(ruta.c_str(), "wb");
        if (f) {
            fwrite(buffer, 1, tam, f);
            fclose(f);
            Medio m(getId(), idUsuario, "foto", "manual", ruta);
            MedioDAO::insertar(bd, m);
            Logger::ok("Foto guardada: " + ruta);
        }
    }
    delete[] buffer;   // buffer temporal de la foto
}

void HiloESP32::iniciarGrabacion(const string& origen, int idUsuario) {
    pthread_mutex_lock(&mutexGrabacion);
    if (!grabando) {              // solo una grabación a la vez
        grabando = true;
        framesGrabados = 0;
        origenGrabacion = origen;
        usuarioGrabacion = idUsuario;
        dirGrabacion = dirMedios + "/tmp_" + selloUnico();
        mkdir(dirGrabacion.c_str(), 0755);
        Logger::ok("Grabación iniciada (" + origen + ")");
    }
    pthread_mutex_unlock(&mutexGrabacion);
}

void HiloESP32::pararGrabacion() {
    pthread_mutex_lock(&mutexGrabacion);
    if (!grabando) { pthread_mutex_unlock(&mutexGrabacion); return; }
    grabando = false;
    string dir = dirGrabacion;
    string origen = origenGrabacion;
    int usuario = usuarioGrabacion;
    int n = framesGrabados;
    pthread_mutex_unlock(&mutexGrabacion);

    if (n < 2) { system(("rm -rf " + dir).c_str()); return; }   // clip vacío, descartar

    // Codificar el mp4 en un HILO APARTE (detached) para no bloquear al hilo de IA:
    // ffmpeg tarda 1-2 s y, si lo hiciéramos aquí, el seguimiento se congelaría ese rato.
    DatosCodificacion* d = new DatosCodificacion{
        bd, getId(), usuario, origen, dir,
        dirMedios + "/video_" + selloUnico() + ".mp4" };
    pthread_t hilo;
    pthread_create(&hilo, nullptr, codificarVideo, d);
    pthread_detach(hilo);   // no hacemos join: el hilo se limpia solo al terminar
}

bool HiloESP32::estaGrabandoIA() {
    pthread_mutex_lock(&mutexGrabacion);
    bool r = grabando && origenGrabacion == "ia";
    pthread_mutex_unlock(&mutexGrabacion);
    return r;
}

void HiloESP32::solicitarCierre() {
    // Para echar a un duplicado: cortamos el socket para que el bucle lector salga.
    conectado = false;
    shutdown(socketFd, SHUT_RDWR);
}

int HiloESP32::getId()             { return esp32 ? esp32->getId() : -1; }
string HiloESP32::getNombre() { return esp32 ? esp32->getNombre() : ""; }
