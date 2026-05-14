#ifndef LISTENERCLIENTES_H
#define LISTENERCLIENTES_H

#include "GestorBD.h"
#include "GestorControl.h"

// metodos/funciones 
// @param entrada 1 puerto TCP en el que escuchar a la app (int)
// @param entrada 2 referencia al Gestor de Hardware para enviar órdenes manuales a los servos
// @param entrada 3 referencia al Gestor de Base de Datos para el login o consultar vídeos
// @param devuelve void
// @funcion principal Levanta un socket TCP paralelo, atiende a los usuarios de la App Flutter y enruta sus peticiones (JSON/Texto).
// @autor ignacio arenas ramos
void hebraListenerClientes(int puerto, GestorControl& gestor_hw, GestorBD& gestor_bd);

#endif