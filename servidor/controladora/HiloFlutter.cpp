#include "HiloFlutter.h"
#include "HiloESP32.h"
#include "GestorConexiones.h"
#include "Usuario.h"
#include "UsuarioDAO.h"
#include "RegistroUsuario.h"
#include "RegistroUsuarioDAO.h"
#include "RegistroIADAO.h"
#include "MedioDAO.h"
#include "Protocolo.h"
#include "Logger.h"
#include "ConexionBD.h"
#include <sys/select.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>
#include <string>

using namespace std;

// Milisegundos del reloj monotónico (para limitar la tasa de frames).
static long ahoraMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}
static const long MS_POR_FRAME = 83;   // ~12 fotogramas por segundo hacia la app

// Lee un entero de 2 / 4 bytes big-endian desde el payload.
static int leerU16(const unsigned char* p) { return (p[0] << 8) | p[1]; }
static int leerU32(const unsigned char* p) { return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }

HiloFlutter::HiloFlutter(int socketFd, ConexionBD* bd, GestorConexiones* gestor, const string& rutaLog) {
    this->socketFd = socketFd;
    this->bd = bd;
    this->gestor = gestor;
    this->rutaLog = rutaLog;
    usuario = nullptr;
    camaraSuscrita = nullptr;
    ultimoContadorEnviado = -1;
    ultimoEnvioMs = 0;
}

HiloFlutter::~HiloFlutter() {
    delete usuario;                 // libera los datos del usuario logueado
    if (socketFd >= 0) close(socketFd);
}

void HiloFlutter::ejecutar() {
    Logger::info("App Flutter conectada");
    while (true) {
        // select con timeout corto: si hay datos del móvil los leemos; si no,
        // aprovechamos para enviarle un frame. Todo en el mismo hilo, sin bloquear.
        fd_set conjunto;
        FD_ZERO(&conjunto);
        FD_SET(socketFd, &conjunto);
        struct timeval espera = {0, 30000};   // 30 ms (~30 fps como máximo)

        int r = select(socketFd + 1, &conjunto, nullptr, nullptr, &espera);
        if (r < 0) break;

        if (r > 0 && FD_ISSET(socketFd, &conjunto)) {
            uint8_t tipo;
            unsigned char* payload;
            uint32_t longitud;
            if (!leerMensaje(socketFd, tipo, payload, longitud)) break;   // desconectado
            procesarComando(tipo, payload, longitud);
            delete[] payload;   // libera el payload reservado por leerMensaje
        }

        enviarFrameSiHayNuevo();
    }
    Logger::warning("App Flutter desconectada");
}

void HiloFlutter::procesarComando(uint8_t tipo, unsigned char* payload, uint32_t longitud) {
    // --- LOGIN: único comando permitido sin estar autenticado ---
    if (tipo == MSG_LOGIN) {
        string texto((char*)payload, longitud);
        size_t sep = texto.find(':');
        string nombre = texto.substr(0, sep);
        string clave  = texto.substr(sep + 1);

        delete usuario;   // por si reintenta el login
        usuario = UsuarioDAO::login(bd, nombre, clave);
        if (usuario) {
            int id = usuario->getId();
            unsigned char p[4] = {
                (unsigned char)((id >> 24) & 0xFF), (unsigned char)((id >> 16) & 0xFF),
                (unsigned char)((id >> 8)  & 0xFF), (unsigned char)( id        & 0xFF) };
            enviarMensaje(socketFd, MSG_LOGIN_OK, p, 4);
            RegistroUsuario r(id, -1, "login");   // -1 = sin cámara
            RegistroUsuarioDAO::insertar(bd, r);
            Logger::ok("Login correcto: " + nombre);
        } else {
            enviarMensaje(socketFd, MSG_LOGIN_FAIL, nullptr, 0);
            Logger::warning("Login fallido: " + nombre);
        }
        return;
    }

    // El resto de comandos exigen estar logueado.
    if (!usuario) return;

    switch (tipo) {
        case MSG_LISTAR_CAMARAS: {
            string lista = gestor->listar();
            enviarMensaje(socketFd, MSG_LISTA_CAMARAS,
                          (const unsigned char*)lista.data(), lista.size());
            break;
        }
        case MSG_SUSCRIBIR: {
            if (longitud < 4) break;
            int id = leerU32(payload);
            HiloESP32* cam = gestor->buscarPorId(id);
            if (cam) {
                camaraSuscrita = cam;
                ultimoContadorEnviado = -1;
                enviarMensaje(socketFd, MSG_SUSCRITO_OK, nullptr, 0);
                RegistroUsuario r(usuario->getId(), id, "acceso_camara");
                RegistroUsuarioDAO::insertar(bd, r);
            } else {
                enviarMensaje(socketFd, MSG_SUSCRITO_FAIL, nullptr, 0);
            }
            break;
        }
        case MSG_SERVO_MANUAL: {
            if (longitud < 4 || !camaraSuscrita) break;
            if (!gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; break; }
            int pan  = leerU16(payload);
            int tilt = leerU16(payload + 2);
            camaraSuscrita->marcarServoManual();   // activa el cooldown de la IA
            camaraSuscrita->enviarServo(pan, tilt);
            RegistroUsuario r(usuario->getId(), camaraSuscrita->getId(), "mover_servo");
            RegistroUsuarioDAO::insertar(bd, r);
            break;
        }
        case MSG_LED_MANUAL: {
            if (longitud < 1 || !camaraSuscrita) break;
            if (!gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; break; }
            camaraSuscrita->enviarLed(payload[0] == 1);
            RegistroUsuario r(usuario->getId(), camaraSuscrita->getId(), "led");
            RegistroUsuarioDAO::insertar(bd, r);
            break;
        }
        case MSG_PEDIR_REGISTROS_IA: {
            int id = (longitud >= 4) ? leerU32(payload) : 0;
            string texto = RegistroIADAO::listar(bd, id);
            enviarMensaje(socketFd, MSG_REGISTROS_IA,
                          (const unsigned char*)texto.data(), texto.size());
            break;
        }
        case MSG_PEDIR_REGISTROS_USUARIO: {
            string texto = RegistroUsuarioDAO::listar(bd);
            enviarMensaje(socketFd, MSG_REGISTROS_USUARIO,
                          (const unsigned char*)texto.data(), texto.size());
            break;
        }
        case MSG_CAPTURAR_FOTO: {
            if (!camaraSuscrita || !gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; break; }
            camaraSuscrita->capturarFoto(usuario->getId());
            break;
        }
        case MSG_INICIAR_GRABACION: {
            if (!camaraSuscrita || !gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; break; }
            camaraSuscrita->iniciarGrabacion("manual", usuario->getId());
            break;
        }
        case MSG_PARAR_GRABACION: {
            if (!camaraSuscrita || !gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; break; }
            camaraSuscrita->pararGrabacion();
            break;
        }
        case MSG_LISTAR_MEDIOS: {
            int id = (longitud >= 4) ? leerU32(payload) : 0;
            string texto = MedioDAO::listar(bd, id);
            enviarMensaje(socketFd, MSG_LISTA_MEDIOS,
                          (const unsigned char*)texto.data(), texto.size());
            break;
        }
        case MSG_PEDIR_MEDIO: {
            if (longitud < 4) break;
            enviarMedio(leerU32(payload));
            break;
        }
        case MSG_PEDIR_LOGS: {
            enviarLogs();
            break;
        }
    }
}

void HiloFlutter::enviarFrameSiHayNuevo() {
    if (!camaraSuscrita) return;
    // La cámara pudo desconectarse: comprobamos que sigue en el gestor.
    if (!gestor->existe(camaraSuscrita)) { camaraSuscrita = nullptr; return; }

    // Límite de tasa: no enviamos más de ~12 fps aunque lleguen más frames
    // (estabiliza el vídeo en la app y reduce el ancho de banda).
    long ahora = ahoraMs();
    if (ahora - ultimoEnvioMs < MS_POR_FRAME) return;

    long contador = camaraSuscrita->getContadorFrame();
    if (contador == ultimoContadorEnviado) return;   // todavía no hay frame nuevo

    unsigned char* buffer = new unsigned char[MAX_PAYLOAD];
    int tam = camaraSuscrita->copiarUltimoFrame(buffer, MAX_PAYLOAD);
    if (tam > 0) enviarMensaje(socketFd, MSG_FRAME_FLUTTER, buffer, tam);
    delete[] buffer;   // buffer temporal del frame a reenviar

    ultimoContadorEnviado = contador;
    ultimoEnvioMs = ahora;
}

// Manda un archivo (foto o vídeo) por trozos: INICIO (tipo+tamaño) -> TROZO* -> FIN.
void HiloFlutter::enviarMedio(int idMedio) {
    string ruta = MedioDAO::obtenerRuta(bd, idMedio);
    if (ruta.empty()) return;
    FILE* f = fopen(ruta.c_str(), "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long total = ftell(f);
    fseek(f, 0, SEEK_SET);

    // tipo: 1 = vídeo (.mp4), 0 = foto
    bool esVideo = ruta.size() > 4 && ruta.substr(ruta.size() - 4) == ".mp4";
    unsigned char cab[5] = { (unsigned char)(esVideo ? 1 : 0),
        (unsigned char)((total >> 24) & 0xFF), (unsigned char)((total >> 16) & 0xFF),
        (unsigned char)((total >> 8)  & 0xFF), (unsigned char)( total        & 0xFF) };
    enviarMensaje(socketFd, MSG_MEDIO_INICIO, cab, 5);

    unsigned char trozo[16384];
    size_t leidos;
    while ((leidos = fread(trozo, 1, sizeof(trozo), f)) > 0) {
        if (!enviarMensaje(socketFd, MSG_MEDIO_TROZO, trozo, leidos)) break;
    }
    fclose(f);
    enviarMensaje(socketFd, MSG_MEDIO_FIN, nullptr, 0);
}

// Manda las últimas líneas del log, quitando el ruido de NNPACK y los colores ANSI.
void HiloFlutter::enviarLogs() {
    string cmd = "grep -av NNPACK \"" + rutaLog +
        "\" 2>/dev/null | sed -r 's/\\x1b\\[[0-9;]*m//g' | tail -n 120";
    FILE* p = popen(cmd.c_str(), "r");
    string texto;
    if (p) {
        char b[512];
        size_t n;
        while ((n = fread(b, 1, sizeof(b), p)) > 0) texto.append(b, n);
        pclose(p);
    }
    enviarMensaje(socketFd, MSG_LOGS, (const unsigned char*)texto.data(), texto.size());
}
