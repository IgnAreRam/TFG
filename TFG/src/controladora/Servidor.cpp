#include "Servidor.h"
#include "ListenerESP32.h"
#include <thread>
#include <iostream>

using namespace std;

Servidor::Servidor(int p_control) : puerto_control(p_control), ejecutando(false) {}

void Servidor::iniciar() {
    cout << "[SYS] Iniciando Servidor Central..." << endl;
    
    int sock = inicializarSocketMaestro(puerto_control);
    if (sock == -1) return;

    ejecutando = true;

    // Lanzamos la hebra y la dejamos trabajar
    thread t_listener(hebraListenerESP32, sock, std::ref(gestor));
    t_listener.detach();

    cout << "[SYS] Servicios en marcha correctamente." << endl;
}