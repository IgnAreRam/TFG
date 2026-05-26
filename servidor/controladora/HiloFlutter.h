#ifndef HILOFLUTTER_H
#define HILOFLUTTER_H

#include <cstdint>
#include <string>

class Usuario;
class HiloESP32;
class ConexionBD;
class GestorConexiones;

/**
 * @brief Gestiona una conexión de la app Flutter.
 *        Corre en un solo hilo. Con select() reparte el tiempo entre:
 *        atender comandos del móvil y reenviarle los frames de la cámara suscrita.
 * @author Ignacio Arenas Ramos
 */
class HiloFlutter {
private:
    int socketFd;
    // Dependencias inyectadas (creadas en main, NO globales):
    ConexionBD* bd;
    GestorConexiones* gestor;
    std::string rutaLog;          // ruta del log del servidor (para el visor de logs)

    Usuario* usuario;             // tras el login (new/delete)
    HiloESP32* camaraSuscrita;    // cámara a la que ve ahora (puntero ajeno, no se libera aquí)
    long ultimoContadorEnviado;   // último frame ya reenviado, para no repetir
    long ultimoEnvioMs;           // momento del último frame enviado (para limitar a ~12 fps)

    void procesarComando(uint8_t tipo, unsigned char* payload, uint32_t longitud);
    void enviarFrameSiHayNuevo();
    void enviarMedio(int idMedio);   // manda un archivo (foto/vídeo) por trozos
    void enviarLogs();               // manda las últimas líneas del log del servidor

public:
    HiloFlutter(int socketFd, ConexionBD* bd, GestorConexiones* gestor, const std::string& rutaLog);
    ~HiloFlutter();

    void ejecutar();
};

#endif
