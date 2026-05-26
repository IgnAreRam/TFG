#ifndef SERVICIOIA_H
#define SERVICIOIA_H

#include <string>
#include <cstdio>
#include <pthread.h>

/**
 * @brief Resultado de analizar un frame con YOLO.
 */
struct Deteccion {
    bool detectado;
    std::string tipo;   // "persona", "perro"...
    int cx;             // centro X del objeto (px, sobre frame 320x240)
    int cy;             // centro Y del objeto
    double confianza;
};

/**
 * @brief Lanza un único proceso Python (yolo_server.py) y le manda los frames
 *        por una tubería (pipe). Como YOLO es único y lo comparten todos los
 *        hilos IA, cada análisis se protege con un mutex (se turnan).
 * @author Ignacio Arenas Ramos
 */
class ServicioIA {
private:
    int   pipeEntrada;     // fd para escribir el frame al Python (su stdin)
    FILE* salidaPython;    // FILE* para leer el JSON del Python (su stdout)
    pid_t pidPython;
    pthread_mutex_t mutex; // un solo hilo usa YOLO a la vez

public:
    ServicioIA();
    ~ServicioIA();

    bool iniciar(const std::string& script);
    Deteccion analizar(const unsigned char* jpeg, int tam);
};

#endif
