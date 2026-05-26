#ifndef ESP32DAO_H
#define ESP32DAO_H

#include <string>
#include "Esp32.h"

class ConexionBD;

/**
 * @brief Acceso a la tabla esp32.
 * @author Ignacio Arenas Ramos
 */
class Esp32DAO {
public:
    // Devuelve un Esp32 nuevo si nombre+clave son válidos Y la cámara está activa.
    // nullptr si no. El llamante debe hacer delete del puntero.
    static Esp32* autenticar(ConexionBD* bd, const std::string& nombre, const std::string& clave);
};

#endif
