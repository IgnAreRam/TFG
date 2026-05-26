#ifndef HILOESP32_H
#define HILOESP32_H

#include <string>
#include <atomic>
#include <ctime>
#include <pthread.h>

class Esp32;
class ConexionBD;
class GestorConexiones;
class ServicioIA;

/**
 * @brief Gestiona una cámara ESP32 conectada.
 *        Corre en su propio hilo (lector) y lanza un segundo hilo (IA).
 *
 *        Hilo lector: recibe los frames JPEG y los deja en un buffer compartido.
 *        Hilo IA: coge el último frame, lo manda a YOLO, mueve los servos para
 *        perseguir al objetivo y guarda un registro en la BD.
 * @author Ignacio Arenas Ramos
 */
class HiloESP32 {
private:
    int socketFd;
    // Dependencias inyectadas (creadas en main, NO globales):
    ConexionBD* bd;
    GestorConexiones* gestor;
    ServicioIA* ia;
    std::string dirMedios;        // carpeta donde guardar fotos/vídeos

    Esp32* esp32;                 // datos de la cámara autenticada (new/delete)
    bool conectado;

    // ── Buffer de frame compartido entre el hilo lector, el de IA y los HiloFlutter ──
    unsigned char* ultimoFrame;   // último JPEG recibido (new[]/delete[])
    int  tamUltimoFrame;
    long contadorFrame;           // sube con cada frame nuevo; sirve para saber si hay uno sin procesar
    pthread_mutex_t mutexFrame;
    pthread_cond_t  condFrame;    // avisa al hilo IA de que hay frame nuevo

    pthread_mutex_t mutexEnvio;   // serializa escrituras al socket (las usan IA y Flutter)
    std::atomic<time_t> ultimoServoManual;   // momento del último servo manual (cooldown IA)

    int panActual;
    int tiltActual;
    pthread_t hiloIA;

    // --- Grabación de vídeo / captura de foto ---
    bool grabando;
    std::string dirGrabacion;       // carpeta temporal con los frames del clip
    int framesGrabados;
    std::string origenGrabacion;    // "manual" | "ia"
    int usuarioGrabacion;           // -1 si la grabación es de la IA
    time_t ultimaDeteccionIA;       // para parar el clip de IA tras unos segundos
    pthread_mutex_t mutexGrabacion; // protege el estado de grabación

    bool autenticar();
    void guardarFrame(const unsigned char* datos, int tam);
    void bucleIA();
    void ajustarServos(int cx, int cy);
    bool estaGrabandoIA();          // ¿hay un clip de IA en curso?
    static void* funcionIA(void* arg);   // punto de entrada del hilo IA

public:
    HiloESP32(int socketFd, ConexionBD* bd, GestorConexiones* gestor,
              ServicioIA* ia, const std::string& dirMedios);
    ~HiloESP32();

    void ejecutar();   // bucle lector (lo llama el hilo creado en main)

    void enviarServo(int pan, int tilt);   // la usan el hilo IA y los HiloFlutter
    void enviarLed(bool encendido);
    void marcarServoManual();              // activa el cooldown de 8s de la IA

    void capturarFoto(int idUsuario);                         // guarda el último frame como .jpg
    void iniciarGrabacion(const std::string& origen, int idUsuario);
    void pararGrabacion();                                    // codifica el clip a .mp4 con ffmpeg
    void solicitarCierre();                                   // echa a un duplicado: corta su socket

    int  copiarUltimoFrame(unsigned char* destino, int maxTam);  // para reenviar a Flutter
    long getContadorFrame();
    int  getId();
    std::string getNombre();
};

#endif
