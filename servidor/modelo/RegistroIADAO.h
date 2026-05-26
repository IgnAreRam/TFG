#ifndef REGISTROIADAO_H
#define REGISTROIADAO_H

#include <string>
#include "RegistroIA.h"

class ConexionBD;

/**
 * @brief Acceso a la tabla registro_ia.
 * @author Ignacio Arenas Ramos
 */
class RegistroIADAO {
public:
    static void insertar(ConexionBD* bd, RegistroIA& registro);

    // Devuelve los registros como texto, una línea por registro:
    // "id_camara;fecha_hora;tipo;confianza". Si idEsp32==0 devuelve todos.
    static std::string listar(ConexionBD* bd, int idEsp32);
};

#endif
