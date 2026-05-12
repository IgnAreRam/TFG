#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "GestorControl.h"
#include <thread>

using namespace std;

// variables // que almaceno y funcionalidad principal
// GestorControl gestor: el cerebro que maneja las cámaras.
// int puerto_control: el puerto donde escuchamos a los ESP32.
// bool ejecutando: flag de estado del sistema.

class Servidor {
private:
    GestorControl gestor;
    int puerto_control;
    bool ejecutando;

public:
    // metodos/funciones 
    // @param entrada 1 p_control: puerto para las cámaras.
    // @param devuelve instancia de Servidor.
    // @funcion principal Constructor del orquestador del sistema.
    // @autor ignacio arenas ramos
    Servidor(int p_control);

    // metodos/funciones 
    // @param devuelve void.
    // @funcion principal Arranca el socket maestro y lanza la hebra listener.
    // @autor ignacio arenas ramos
    void iniciar();
};

#endif