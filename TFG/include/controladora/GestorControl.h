#ifndef GESTORCONTROL_H
#define GESTORCONTROL_H

#include <map>
#include <mutex>
#include <vector>
#include "CamaraESP.h"

using namespace std;

// variables //que almaceno y funcionalidad principal 
// map<int, CamaraESP*> lista_camaras: mapa que asocia el ID de base de datos con el objeto de la cámara.
// mutex mtx_lista: semáforo para evitar condiciones de carrera al acceder al mapa desde diferentes hebras.
// Funcionalidad principal: Centralizar el acceso y estado de todas las cámaras conectadas al servidor.

class GestorControl {
private:
    map<int, CamaraESP*> lista_camaras;
    mutex mtx_lista;

public:
    // metodos/funciones 
    // @param entrada 1 el objeto camara (CamaraESP*)
    // @param devuelve void
    // @funcion principal añade una nueva cámara al mapa de forma segura usando el mutex
    // @autor ignacio arenas ramos
    void agregarCamara(CamaraESP* nueva_camara);

    // metodos/funciones 
    // @param entrada 1 el id de la camara (int)
    // @param devuelve void
    // @funcion principal busca la cámara por ID y llama a su método registrarLatido()
    // @autor ignacio arenas ramos
    void procesarPing(int id_buscado);

    // metodos/funciones 
    // @param entrada 1 el tiempo limite de espera (int segundos)
    // @param devuelve void
    // @funcion principal recorre el mapa y marca como OFFLINE a las cámaras que no han enviado PING hace X segundos
    // @autor ignacio arenas ramos
    void revisarConexiones(int timeout_seg);
};

#endif