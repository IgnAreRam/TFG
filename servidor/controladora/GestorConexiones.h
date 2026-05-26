#ifndef GESTORCONEXIONES_H
#define GESTORCONEXIONES_H

#include <vector>
#include <string>
#include <pthread.h>

class HiloESP32;   // declaración adelantada: solo guardamos punteros

/**
 * @brief Lleva la lista de cámaras ESP32 conectadas en este momento.
 *        Lo usan los HiloFlutter para buscar cámaras y suscribirse a ellas.
 *        La lista la tocan varios hilos, así que va protegida por un mutex.
 * @author Ignacio Arenas Ramos
 */
class GestorConexiones {
private:
    std::vector<HiloESP32*> camaras;
    pthread_mutex_t mutex;

public:
    GestorConexiones();
    ~GestorConexiones();

    void registrar(HiloESP32* camara);
    void eliminar(HiloESP32* camara);
    HiloESP32* buscarPorId(int id);     // nullptr si no está conectada
    bool existe(HiloESP32* camara);     // ¿sigue conectada esta cámara?
    std::string listar();               // "id;nombre;1\n..." (1 = conectada)
};

#endif
