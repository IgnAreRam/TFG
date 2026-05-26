#include "Esp32DAO.h"
#include "ConexionBD.h"

using namespace std;

Esp32* Esp32DAO::autenticar(ConexionBD* bd, const string& nombre, const string& clave) {
    PGresult* res = bd->consultar(
        "SELECT id, activa FROM esp32 WHERE nombre=$1 AND clave=$2",
        {nombre, clave});

    Esp32* encontrado = nullptr;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1) {
        int id = atoi(PQgetvalue(res, 0, 0));
        bool activa = (PQgetvalue(res, 0, 1)[0] == 't');   // PostgreSQL devuelve 't'/'f'
        if (activa) encontrado = new Esp32(id, nombre, true);   // lo libera el HiloESP32
    }
    PQclear(res);
    return encontrado;
}
