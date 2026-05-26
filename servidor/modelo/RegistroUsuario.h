#ifndef REGISTROUSUARIO_H
#define REGISTROUSUARIO_H

#include <string>

/**
 * @brief Acción de un operador (login, acceso a cámara, mover servo, led).
 *        idEsp32 = -1 cuando la acción no implica cámara (por ejemplo el login).
 * @author Ignacio Arenas Ramos
 */
class RegistroUsuario {
private:
    int idUsuario;
    int idEsp32;          // -1 = sin cámara (se guardará como NULL)
    std::string accion;

public:
    RegistroUsuario(int idUsuario, int idEsp32, const std::string& accion);

    int getIdUsuario();
    int getIdEsp32();
    std::string getAccion();
};

#endif
