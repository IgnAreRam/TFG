#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "GestorControl.h"
#include "ColaFrames.h"
#include "GestorBD.h"
#include "ProcesadorIA.h"
#include <atomic>

using namespace std;

// variables //que almaceno y funcionalidad principal 
// int puerto_camaras: puerto tcp exclusivo para que el hardware (ESP32) envíe vídeo y reciba órdenes de servos.
// int puerto_clientes: puerto tcp exclusivo para que la app (Flutter) pida vídeos y mueva la cámara manualmente.
// GestorBD gestor_bd: El conector con PostgreSQL.
// ColaFrames cola_video: El buffer de la IA.
// ProcesadorIA ia: El cerebro que busca personas.
// Funcionalidad principal: Orquestar todo el sistema, iniciar los puertos y repartir los gestores entre las hebras.

class Servidor {
private:
    int puerto_camaras;
    int puerto_clientes; 
    atomic<bool> activo;

    GestorControl gestor_hardware;
    ColaFrames cola_video;
    
    // Usamos punteros para instanciarlos cuando el servidor arranque y la BD esté lista
    GestorBD* gestor_bd;
    ProcesadorIA* ia;

public:
    // metodos/funciones 
    // @param entrada 1 puerto para el hardware ESP32 (int)
    // @param entrada 2 puerto para la app Flutter (int)
    // @param devuelve Instancia de Servidor
    // @funcion principal Constructor del orquestador.
    // @autor ignacio arenas ramos
    Servidor(int p_camaras, int p_clientes);

    // metodos/funciones 
    // @param devuelve void
    // @funcion principal Arranca la BD, la IA y levanta los dos listeners (Cámaras y Clientes) en hebras separadas.
    // @autor ignacio arenas ramos
    void ejecutar();
};

#endif