#ifndef USUARIO_H
#define USUARIO_H

#include <string>

/**
 * @brief Operador que usa la app Flutter.
 * @author Ignacio Arenas Ramos
 */
class Usuario {
private:
    int id;
    std::string nombre;

public:
    Usuario(int id, const std::string& nombre);

    int getId();
    std::string getNombre();
};

#endif
