#ifndef REGISTROUSUARIODAO_H
#define REGISTROUSUARIODAO_H

#include <string>
#include "RegistroUsuario.h"

class ConexionBD;

/**
 * @brief Acceso a la tabla registro_usuario.
 * @author Ignacio Arenas Ramos
 */
class RegistroUsuarioDAO {
public:
    static void insertar(ConexionBD* bd, RegistroUsuario& registro);

    // Devuelve los registros como texto: "usuario;fecha_hora;accion;id_camara".
    static std::string listar(ConexionBD* bd);
};

#endif
