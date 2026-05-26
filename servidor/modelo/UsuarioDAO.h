#ifndef USUARIODAO_H
#define USUARIODAO_H

#include <string>
#include "Usuario.h"

class ConexionBD;

/**
 * @brief Acceso a la tabla usuario.
 * @author Ignacio Arenas Ramos
 */
class UsuarioDAO {
public:
    // Devuelve un Usuario nuevo si las credenciales son válidas, o nullptr.
    // El llamante debe hacer delete del puntero devuelto.
    static Usuario* login(ConexionBD* bd, const std::string& usuario, const std::string& contrasena);
};

#endif
