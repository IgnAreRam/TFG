#include "ListenerESP32.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <iostream>

using namespace std;

// variables // que almaceno y funcionalidad principal
// int opt: opción para configurar el socket.
// struct sockaddr_in address: estructura para la dirección IP y puerto.
// int flags: banderas de configuración del descriptor de archivo.

// metodos/funciones 
// @param entrada 1 puerto: entero que define el puerto de escucha.
// @param devuelve un entero con el descriptor del socket maestro.
// @funcion principal Crea un socket, permite la reutilización de puerto, lo pone en modo no bloqueante y lo prepara para escuchar conexiones.
// @autor ignacio arenas ramos
int inicializarSocketMaestro(int puerto) {
    int fd;
    int opt = 1;
    struct sockaddr_in address;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket fallido");
        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    int flags = fcntl(fd, f_GETFL, 0);
    fcntl(fd, f_SETFL, flags | O_NONBLOCK);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(puerto);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind fallido");
        return -1;
    }

    if (listen(fd, 10) < 0) {
        perror("Listen fallido");
        return -1;
    }

    return fd;
}

// variables // que almaceno y funcionalidad principal
// fd_set set_lectura: conjunto de descriptores para monitorizar.
// int nuevo_socket: descriptor para la conexión aceptada.
// CamaraESP* nueva_camara: puntero al objeto que gestionará la sesión.

// metodos/funciones 
// @param entrada 1 socket_maestro: el descriptor del socket principal.
// @param entrada 2 gestor: referencia al objeto GestorControl para registrar cámaras.
// @param devuelve void.
// @funcion principal Bucle de servicio que usa select() para aceptar conexiones de ESP32 sin bloquear la CPU.
// @autor ignacio arenas ramos
void hebraListenerESP32(int socket_maestro, GestorControl& gestor) {
    fd_set set_lectura;
    struct sockaddr_in direccion;
    socklen_t addrlen = sizeof(direccion);

    while (true) {
        FD_ZERO(&set_lectura);
        FD_SET(socket_maestro, &set_lectura);

        int actividad = select(socket_maestro + 1, &set_lectura, NULL, NULL, NULL);

        if (actividad < 0) continue;

        if (FD_ISSET(socket_maestro, &set_lectura)) {
            int nuevo_socket = accept(socket_maestro, (struct sockaddr *)&direccion, &addrlen);
            if (nuevo_socket > 0) {
                // Simulación inicial de ID: En el futuro leeremos el protocolo inicial
                int id_simulado = 1; 
                CamaraESP* nueva_camara = new CamaraESP(id_simulado, nuevo_socket);
                gestor.agregarCamara(nueva_camara);
                cout << "[NET] ESP32 conectado. Socket: " << nuevo_socket << endl;
            }
        }
    }

    if (FD_ISSET(socket_maestro, &set_lectura)) {
            int nuevo_socket = accept(socket_maestro, (struct sockaddr *)&direccion, &addrlen);
            
            if (nuevo_socket > 0) {
                // 1. En lugar de simular, leemos el socket
                int id_real = Protocolo::leerIdentificacion(nuevo_socket);

                if (id_real != -1) {
                    // 2. Si es válido, instanciamos la cámara con su ID real
                    CamaraESP* nueva_camara = new CamaraESP(id_real, nuevo_socket);
                    gestor.agregarCamara(nueva_camara);
                    cout << "[NET] ESP32 Registrado con EXITO. ID: " << id_real << endl;
                } else {
                    // 3. Si manda basura o falla, cerramos la conexión por seguridad
                    cout << "[NET] Intento de conexion invalido. Rechazando..." << endl;
                    close(nuevo_socket); 
                }
            }
        }
}