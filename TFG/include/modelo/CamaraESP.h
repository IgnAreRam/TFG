#include <iostream>
#include <ctime>

using namespace std;

// variables //que almaceno y funcionalidad principal 
// int id_camara: almacena el ID de bd. 
// int socket_fd: almacena el socket de control.
// time_t ultimo_ping: almacena el timestamp. 
// bool online: almacena el estado. 
// Funcionalidad principal: Representar la conexion de control persistente de una camara.

class CamaraESP {
private:
    int id_camara;
    int socket_fd;
    time_t ultimo_ping;
    bool online;

public:
    // metodos/funciones 
    // @param entrada 1 el parametro de entrada es un objetivo (id_camara entero)
    // @param entrada 2 el parametro de entrada es un objetivo (descriptor socket entero)
    // @param devuelve un objeto con el resultado de inicializar la sesion de la camara
    // @funcion principal este metodo/function sirve para construir la instancia del ESP32 cuando se conecta, guardando su socket y marcando el tiempo actual como su primer ping
    // @autor ignacio arenas ramos
    CamaraESP(int id, int fd) {
        id_camara = id;
        socket_fd = fd;
        ultimo_ping = time(nullptr);
        online = true;
    }

    // metodos/funciones 
    // @param entrada 1 el parametro de entrada es un objetivo vacio (void)
    // @param devuelve un objeto vacio (void) actualizando el estado interno
    // @funcion principal este metodo/function sirve para actualizar la variable ultimo_ping al momento actual cuando el Listener recibe el mensaje PING por el socket
    // @autor ignacio arenas ramos
    void registrarLatido() {
        ultimo_ping = time(nullptr);
        online = true;
    }

    // metodos/funciones 
    // @param entrada 1 un string con el comando exacto a enviar
    // @param devuelve un booleano (true si se envió bien, false si falló)
    // @funcion principal Escribe un mensaje de texto plano en el socket TCP hacia el ESP32.
    // @autor ignacio arenas ramos
    bool enviarComando(const string& comando);

    // Getters básicos (Autodescriptivos, sin alterar la lógica compleja)
    int getId() const { return id_camara; }
    int getSocket() const { return socket_fd; }
    time_t getUltimoPing() const { return ultimo_ping; }
    bool isOnline() const { return online; }
    
    void setOffline() { online = false; }
};