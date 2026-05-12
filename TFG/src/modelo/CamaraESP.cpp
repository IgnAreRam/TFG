#include "CamaraESP.h"

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