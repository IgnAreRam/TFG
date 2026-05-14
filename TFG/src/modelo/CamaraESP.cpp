#include "CamaraESP.h"
#include <sys/socket.h>
#include <cstring>

// metodos/funciones 
// @param entrada 1 id de cámara
// @param entrada 2 descriptor de socket
// @param devuelve el objeto construido
// @funcion principal Inicializa la cámara y marca el tiempo actual como primer contacto
// @autor ignacio arenas ramos
CamaraESP::CamaraESP(int id, int fd) : id_camara(id), socket_fd(fd) {
    this->ultimo_ping = time(nullptr);
    this->online = true;
}

// metodos/funciones 
// @param devuelve void
// @funcion principal pone el timestamp al tiempo actual del sistema
// @autor ignacio arenas ramos
void CamaraESP::registrarLatido() {
    this->ultimo_ping = time(nullptr);
    this->online = true;
}

// Implementación de Getters sencillos
int CamaraESP::getId() const { return id_camara; }
int CamaraESP::getSocket() const { return socket_fd; }
bool CamaraESP::isOnline() const { return online; }
time_t CamaraESP::getUltimoPing() const { return ultimo_ping; }
void CamaraESP::setOffline() { online = false; }

bool CamaraESP::enviarComando(const string& comando) {
    if (!online) {
        cerr << "[AVISO] Intento de mover servo en cámara " << id_camara << " que está OFFLINE." << endl;
        return false;
    }

    // Aseguramos que el comando termine en salto de línea para que el ESP32 sepa dónde acaba
    string mensaje_final = comando + "\n"; 

    // send() es la función nativa de C para escribir en un socket TCP
    int bytes_enviados = send(socket_fd, mensaje_final.c_str(), mensaje_final.length(), 0);

    if (bytes_enviados < 0) {
        cerr << "[ERROR] Fallo al enviar comando a la cámara " << id_camara << endl;
        return false;
    }

    return true;
}