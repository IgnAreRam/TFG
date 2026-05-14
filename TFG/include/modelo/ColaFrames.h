#ifndef COLAFRAMES_H
#define COLAFRAMES_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

using namespace std;

// variables //que almaceno y funcionalidad principal 
// int id_camara: almacena de qué cámara viene la imagen.
// vector<unsigned char> datos_imagen: almacena los bytes crudos del JPG/PNG.
// Funcionalidad principal: Estructura de transporte (Payload) entre la red y la IA.

struct FrameData {
    int id_camara;
    vector<unsigned char> datos_imagen; 
};

// variables //que almaceno y funcionalidad principal 
// queue<FrameData> cola: la estructura de datos estándar (FIFO - First In, First Out).
// mutex mtx: el cerrojo para proteger la cola.
// condition_variable cv: señalizador para despertar a la IA cuando llegan fotogramas nuevos.
// Funcionalidad principal: Actuar como amortiguador (buffer) seguro entre la red y el procesamiento pesado.

class ColaFrames {
private:
    queue<FrameData> cola;
    mutex mtx;
    condition_variable cv;

public:
    // metodos/funciones 
    // @param entrada 1 el frame de datos (FrameData)
    // @param devuelve void
    // @funcion principal Inserta un frame de forma segura en la cola y avisa a la IA.
    // @autor ignacio arenas ramos
    void insertar(const FrameData& frame);

    // metodos/funciones 
    // @param devuelve el frame extraído (FrameData)
    // @funcion principal Extrae el fotograma más antiguo. Si está vacía, bloquea la hebra (duerme) hasta que haya uno.
    // @autor ignacio arenas ramos
    FrameData extraer();
};

#endif