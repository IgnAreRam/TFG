#include "Protocolo.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

using namespace std;

int Protocolo::leerIdentificacion(int socket_fd) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    // Leemos del socket. recv devuelve la cantidad de bytes leídos.
    int bytes_leidos = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_leidos <= 0) {
        cerr << "[ERROR] Fallo al leer identificacion o cliente desconectado." << endl;
        return -1;
    }

    string mensaje(buffer);
    cout << "[PROTOCOLO] Mensaje recibido: " << mensaje << endl;

    // Supongamos que el ESP32 envía "AUTH:4"
    string prefijo = "AUTH:";
    size_t pos = mensaje.find(prefijo);

    if (pos != string::npos) {
        // Extraemos el número que va después de "AUTH:"
        try {
            string str_id = mensaje.substr(pos + prefijo.length());
            int id_camara = stoi(str_id);
            return id_camara;
        } catch (...) {
            cerr << "[ERROR] Formato de ID inválido en el protocolo." << endl;
            return -1;
        }
    }

    return -1; // No se encontró el prefijo esperado
}