#include "ListenerClientes.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <cstring>

using namespace std;

// metodos/funciones 
// @param entrada 1 socket del cliente conectado (int)
// @param entrada 2 referencia al gestor de BD (GestorBD&)
// @param entrada 3 referencia al gestor de hardware (GestorControl&)
// @param devuelve void
// @funcion principal Lee la petición de Flutter, la trocea, ejecuta la acción (BD o Hardware) y devuelve el resultado.
// @autor ignacio arenas ramos
void atenderClienteFlutter(int client_socket, GestorBD& gestor_bd, GestorControl& gestor_hw) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Leemos la petición enviada por la App Flutter
    int bytes_leidos = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_leidos <= 0) {
        close(client_socket);
        return;
    }

    string peticion(buffer);
    cout << "[APP] Petición recibida: " << peticion << endl;

    // Parseamos el comando separando por ':'
    vector<string> tokens;
    stringstream ss(peticion);
    string token;
    while (getline(ss, token, ':')) {
        tokens.push_back(token);
    }

    string respuesta = "ERROR:Comando desconocido\n";

    if (!tokens.empty()) {
        string comando = tokens[0];

        // 1. GESTIÓN DE USUARIOS (Login)
        if (comando == "LOGIN" && tokens.size() == 3) {
            string usuario = tokens[1];
            string password_hash = tokens[2];
            
            // Llamamos a la BD para verificar
            int id_usuario = gestor_bd.verificarLogin(usuario, password_hash); 
            
            if (id_usuario != -1) {
                respuesta = "OK:" + to_string(id_usuario) + "\n";
            } else {
                respuesta = "ERROR:Credenciales invalidas\n";
            }
        }
        // 2. REGISTRO DE USUARIOS
        else if (comando == "REGISTRO" && tokens.size() == 3) {
            string usuario = tokens[1];
            string password_hash = tokens[2];
            
            // Llamamos a la BD para crear usuario
            int id_usuario = gestor_bd.crearUsuario(usuario, password_hash);
            
            if (id_usuario != -1) {
                respuesta = "OK:" + to_string(id_usuario) + "\n";
            } else {
                respuesta = "ERROR:No se pudo crear el usuario\n";
            }
        }
        // 3. CONTROL DE HARDWARE (Mover cámara manual)
        else if (comando == "MOVER" && tokens.size() == 4) {
            try {
                int id_cam = stoi(tokens[1]);
                string eje = tokens[2];
                int grados = stoi(tokens[3]);
                
                // Llamamos al gestor de hardware
                bool exito = gestor_hw.moverServo(id_cam, eje, grados);
                
                if (exito) respuesta = "OK:Movimiento ejecutado\n";
                else respuesta = "ERROR:Fallo al mover la camara\n";
            } catch (...) {
                respuesta = "ERROR:Formato de movimiento invalido\n";
            }
        }
        // (Aquí podrías añadir más comandos en el futuro, como pedir el historial de intrusos)
    }

    // Enviamos la respuesta de vuelta a la App Flutter
    send(client_socket, respuesta.c_str(), respuesta.length(), 0);
    
    // Cerramos el socket (conexión de un solo uso para no saturar el servidor)
    close(client_socket);
}

// metodos/funciones 
// @param entrada 1 puerto TCP en el que escuchar a la app (int)
// @param entrada 2 referencia al Gestor de Hardware para enviar órdenes manuales a los servos
// @param entrada 3 referencia al Gestor de Base de Datos para el login o consultar vídeos
// @param devuelve void
// @funcion principal Levanta un socket TCP paralelo, atiende a los usuarios de la App Flutter y enruta sus peticiones.
// @autor ignacio arenas ramos
void hebraListenerClientes(int puerto, GestorControl& gestor_hw, GestorBD& gestor_bd) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // 1. Creamos el socket maestro para la App
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "[FATAL APP] Fallo al crear socket para clientes." << endl;
        return;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Escucha en todas las IPs del servidor
    address.sin_port = htons(puerto);

    // 2. Vinculamos y escuchamos
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "[FATAL APP] Fallo en el bind del puerto " << puerto << endl;
        return;
    }

    if (listen(server_fd, 10) < 0) {
        cerr << "[FATAL APP] Fallo al escuchar en el puerto " << puerto << endl;
        return;
    }

    cout << "[NET APP] Servidor listo. Escuchando App Flutter en el puerto " << puerto << "..." << endl;

    // 3. Bucle infinito aceptando conexiones de móviles
    socklen_t addrlen = sizeof(address);
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            cerr << "[ERROR APP] Fallo al aceptar conexión de Flutter." << endl;
            continue;
        }

        // Lanzamos un hilo ligero (thread) para atender a este usuario y no bloquear a los demás
        thread t(atenderClienteFlutter, new_socket, std::ref(gestor_bd), std::ref(gestor_hw));
        t.detach(); // Lo desvinculamos para que se limpie solo al terminar
    }
}