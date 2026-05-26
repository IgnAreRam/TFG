#ifndef MEDIODAO_H
#define MEDIODAO_H

#include <string>
#include "Medio.h"

class ConexionBD;

/**
 * @brief Acceso a la tabla medio.
 * @author Ignacio Arenas Ramos
 */
class MedioDAO {
public:
    static void insertar(ConexionBD* bd, Medio& medio);

    // Lista como texto: "id;tipo;origen;fecha;id_camara". idEsp32==0 -> todos.
    static std::string listar(ConexionBD* bd, int idEsp32);

    // Devuelve la ruta del archivo de un medio (vacío si no existe).
    static std::string obtenerRuta(ConexionBD* bd, int idMedio);
};

#endif
