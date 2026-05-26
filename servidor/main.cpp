/**
 * @brief Punto de entrada de El Centinela.
 *        Arranca BD + YOLO + gestor, escucha en un único puerto TCP y, según
 *        el primer byte de cada cliente, lanza un hilo de ESP32 o de Flutter.
 * @author Ignacio Arenas Ramos
 */
#include "Protocolo.h"
#include "ConexionBD.h"
#include "GestorConexiones.h"
#include "ServicioIA.h"
#include "HiloESP32.h"
#include "HiloFlutter.h"
#include "Logger.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <fstream>
#include <map>
#include <pthread.h>

using namespace std;

static volatile sig_atomic_t ejecutando = 1;
static void manejarSenal(int) { ejecutando = 0; }

// Lee el config (líneas "clave=valor") a un mapa.
static map<string, string> leerConfig(const string& ruta) {
    map<string, string> cfg;
    ifstream f(ruta);
    string linea;
    while (getline(f, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        size_t igual = linea.find('=');
        if (igual == string::npos) continue;
        cfg[linea.substr(0, igual)] = linea.substr(igual + 1);
    }
    return cfg;
}

// Datos que main pasa a cada hilo de conexión (dependencias inyectadas, sin globales).
struct DatosConexion {
    int fd;
    ConexionBD* bd;
    GestorConexiones* gestor;
    ServicioIA* ia;
    string dirMedios;
    string rutaLog;
};

// ── HILO: punto de entrada del hilo que atiende a una cámara ESP32 ──
static void* lanzarESP32(void* arg) {
    DatosConexion* d = (DatosConexion*)arg;
    HiloESP32* hilo = new HiloESP32(d->fd, d->bd, d->gestor, d->ia, d->dirMedios);
    hilo->ejecutar();
    delete hilo;   // libera la cámara (y su buffer) al desconectarse
    delete d;
    return nullptr;
}

// ── HILO: punto de entrada del hilo que atiende a una app Flutter ──
static void* lanzarFlutter(void* arg) {
    DatosConexion* d = (DatosConexion*)arg;
    HiloFlutter* hilo = new HiloFlutter(d->fd, d->bd, d->gestor, d->rutaLog);
    hilo->ejecutar();
    delete hilo;
    delete d;
    return nullptr;
}

int main(int argc, char* argv[]) {
    string rutaConfig = (argc > 1) ? argv[1] : "config.txt";
    auto cfg = leerConfig(rutaConfig);

    // No morir si escribimos en un socket que el cliente ya cerró.
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, manejarSenal);

    // Rutas de medios y log (con valores por defecto si no están en el config).
    string dirMedios = cfg.count("medios") ? cfg["medios"] : "../../medios";
    string rutaLog   = cfg.count("log")    ? cfg["log"]    : "../../servidor.log";

    // Objetos creados aquí, en main. Se pasan por inyección de dependencias
    // a cada hilo (NO son variables globales).
    // 1) Base de datos
    ConexionBD bd(cfg["db"]);
    if (!bd.conectado()) { Logger::error("No se pudo conectar a la BD"); return 1; }

    // 2) Proceso YOLO
    ServicioIA ia;
    if (!ia.iniciar(cfg["yolo"])) { Logger::error("No se pudo lanzar YOLO"); return 1; }

    // 3) Gestor de cámaras
    GestorConexiones gestor;

    // 4) Socket de escucha
    int puerto = atoi(cfg["puerto"].c_str());
    int servidor = socket(AF_INET, SOCK_STREAM, 0);
    int opcion = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));

    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_addr.s_addr = INADDR_ANY;
    dir.sin_port = htons(puerto);

    if (bind(servidor, (struct sockaddr*)&dir, sizeof(dir)) < 0) {
        Logger::error("No se pudo abrir el puerto " + to_string(puerto));
        return 1;
    }
    listen(servidor, 8);
    Logger::ok("El Centinela escuchando en el puerto " + to_string(puerto));

    // 5) Bucle de aceptación
    while (ejecutando) {
        int cliente = accept(servidor, nullptr, nullptr);
        if (cliente < 0) continue;   // accept interrumpido (p.ej. por SIGINT)

        // Miramos el primer byte SIN consumirlo (MSG_PEEK) para saber quién es.
        unsigned char primerByte = 0;
        if (recv(cliente, &primerByte, 1, MSG_PEEK) <= 0) { close(cliente); continue; }

        pthread_t hilo;
        DatosConexion* arg = new DatosConexion{cliente, &bd, &gestor, &ia, dirMedios, rutaLog};
        if (primerByte == MSG_ESP32_AUTH) {
            pthread_create(&hilo, nullptr, lanzarESP32, arg);
            pthread_detach(hilo);    // no hacemos join: el hilo se limpia solo al terminar
        } else if (primerByte == MSG_LOGIN) {
            pthread_create(&hilo, nullptr, lanzarFlutter, arg);
            pthread_detach(hilo);
        } else {
            delete arg;
            close(cliente);          // cliente desconocido: lo rechazamos
        }
    }

    // 6) Cierre ordenado (bd, ia y gestor se destruyen solos al salir de main)
    Logger::info("Cerrando El Centinela...");
    close(servidor);
    return 0;
}
