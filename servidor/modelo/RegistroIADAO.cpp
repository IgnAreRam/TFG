#include "RegistroIADAO.h"
#include "ConexionBD.h"

using namespace std;

void RegistroIADAO::insertar(ConexionBD* bd, RegistroIA& registro) {
    bd->ejecutar(
        "INSERT INTO registro_ia (id_esp32, tipo, confianza) VALUES ($1, $2, $3)",
        { to_string(registro.getIdEsp32()),
          registro.getTipo(),
          to_string(registro.getConfianza()) });
}

string RegistroIADAO::listar(ConexionBD* bd, int idEsp32) {
    // Un único SQL: si idEsp32 es 0 ($1=0) la condición OR deja pasar todas las filas.
    PGresult* res = bd->consultar(
        "SELECT id_esp32, fecha_hora, tipo, confianza FROM registro_ia "
        "WHERE ($1 = 0 OR id_esp32 = $1) ORDER BY fecha_hora DESC LIMIT 100",
        { to_string(idEsp32) });

    string texto;
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        for (int i = 0; i < PQntuples(res); i++) {
            texto += PQgetvalue(res, i, 0); texto += ';';
            texto += PQgetvalue(res, i, 1); texto += ';';
            texto += PQgetvalue(res, i, 2); texto += ';';
            texto += PQgetvalue(res, i, 3); texto += '\n';
        }
    }
    PQclear(res);
    return texto;
}
