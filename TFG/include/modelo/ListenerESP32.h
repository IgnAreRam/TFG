#ifndef LISTENERESP32_H
#define LISTENERESP32_H

#include "GestorControl.h"

// metodos/funciones 
// @param entrada 1 el parametro de entrada es un puerto (int)
// @param devuelve un entero (int) que es el descriptor del socket configurado o -1 si falla.
// @funcion principal Crea un socket AF_INET, no bloqueante y lo vincula al puerto especificado.
// @autor ignacio arenas ramos
int inicializarSocketMaestro(int puerto);

// metodos/funciones 
// @param entrada 1 el parametro de entrada es un socket_maestro de tipo entero (fd)
// @param entrada 2 el parametro de entrada es un gestor por referencia (GestorControl&)
// @param devuelve un objeto vacio (void)
// @funcion principal Bucle infinito que procesa las conexiones entrantes usando select() y accept().
// @autor ignacio arenas ramos
void hebraListenerESP32(int socket_maestro, GestorControl& gestor);

#endif