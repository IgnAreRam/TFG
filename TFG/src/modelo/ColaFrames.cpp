#include "ColaFrames.h"

// metodos/funciones 
// @param entrada 1 el frame de datos a meter
// @param devuelve void
// @funcion principal Bloquea el mutex, mete el frame y lanza una señal a cv para despertar a quien esté esperando.
// @autor ignacio arenas ramos
void ColaFrames::insertar(const FrameData& frame) {
    {
        // 1. Bloqueamos solo durante la inserción
        lock_guard<mutex> lock(mtx);
        cola.push(frame);
    } // El mutex se libera automáticamente aquí
    
    // 2. Avisamos a UNA hebra que esté esperando (nuestra IA) que ya hay trabajo
    cv.notify_one(); 
}

// metodos/funciones 
// @param devuelve el FrameData extraído
// @funcion principal Bloquea la hebra de la IA si no hay fotogramas. Cuando despierta, saca el fotograma y lo devuelve.
// @autor ignacio arenas ramos
FrameData ColaFrames::extraer() {
    unique_lock<mutex> lock(mtx);
    
    // Si la cola está vacía, el wait() SUELTA el mutex y pone a dormir a la hebra.
    // Cuando recibe el notify_one() de insertar(), se despierta y vuelve a coger el mutex.
    cv.wait(lock, [this]() { return !cola.empty(); });
    
    // Sacamos el fotograma (el más antiguo, FIFO)
    FrameData frame = cola.front();
    cola.pop();
    
    return frame;
}