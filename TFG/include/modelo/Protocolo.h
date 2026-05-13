#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <string>

using namespace std;

// variables // que almaceno y funcionalidad principal
// Este módulo no almacena variables de estado interno.
// Funcionalidad principal: Proporcionar métodos estáticos para codificar y decodificar los mensajes de control entre el Servidor y el ESP32.

namespace Protocolo {
    
    // metodos/funciones 
    // @param entrada 1 el parametro de entrada es un descriptor de socket (int)
    // @param devuelve un entero (int) con el ID de la camara, o -1 si hay error
    // @funcion principal este metodo/function sirve para leer los primeros bytes que envia el ESP32 al conectarse, buscar el patron de autenticacion (ej: "AUTH:4") y extraer el ID real de la base de datos.
    // @autor ignacio arenas ramos
    int leerIdentificacion(int socket_fd);

}

#endif