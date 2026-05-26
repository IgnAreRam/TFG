#include "UsuarioDAO.h"
#include "ConexionBD.h"

using namespace std;

Usuario* UsuarioDAO::login(ConexionBD* bd, const string& usuario, const string& contrasena) {
    // Parámetros $1 y $2: nunca concatenamos texto del usuario en el SQL (evita inyección).
    PGresult* res = bd->consultar(
        "SELECT id FROM usuario WHERE usuario=$1 AND contrasena=$2",
        {usuario, contrasena});

    Usuario* encontrado = nullptr;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1) {
        int id = atoi(PQgetvalue(res, 0, 0));
        encontrado = new Usuario(id, usuario);   // lo libera el HiloFlutter al desconectar
    }
    PQclear(res);
    return encontrado;
}
