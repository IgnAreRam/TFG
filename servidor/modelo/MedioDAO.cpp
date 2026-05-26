#include "MedioDAO.h"
#include "ConexionBD.h"

using namespace std;

void MedioDAO::insertar(ConexionBD* bd, Medio& medio) {
    // idUsuario -1 -> NULL (lo generó la IA).
    if (medio.getIdUsuario() < 0) {
        bd->ejecutar(
            "INSERT INTO medio (id_esp32, id_usuario, tipo, origen, ruta) "
            "VALUES ($1, NULL, $2, $3, $4)",
            { to_string(medio.getIdEsp32()), medio.getTipo(),
              medio.getOrigen(), medio.getRuta() });
    } else {
        bd->ejecutar(
            "INSERT INTO medio (id_esp32, id_usuario, tipo, origen, ruta) "
            "VALUES ($1, $2, $3, $4, $5)",
            { to_string(medio.getIdEsp32()), to_string(medio.getIdUsuario()),
              medio.getTipo(), medio.getOrigen(), medio.getRuta() });
    }
}

string MedioDAO::listar(ConexionBD* bd, int idEsp32) {
    PGresult* res = bd->consultar(
        "SELECT id, tipo, origen, fecha_hora, id_esp32 FROM medio "
        "WHERE ($1 = 0 OR id_esp32 = $1) ORDER BY id DESC LIMIT 100",
        { to_string(idEsp32) });

    string texto;
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        for (int i = 0; i < PQntuples(res); i++) {
            texto += PQgetvalue(res, i, 0); texto += ';';
            texto += PQgetvalue(res, i, 1); texto += ';';
            texto += PQgetvalue(res, i, 2); texto += ';';
            texto += PQgetvalue(res, i, 3); texto += ';';
            texto += PQgetvalue(res, i, 4); texto += '\n';
        }
    }
    PQclear(res);
    return texto;
}

string MedioDAO::obtenerRuta(ConexionBD* bd, int idMedio) {
    PGresult* res = bd->consultar(
        "SELECT ruta FROM medio WHERE id = $1", { to_string(idMedio) });
    string ruta;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1)
        ruta = PQgetvalue(res, 0, 0);
    PQclear(res);
    return ruta;
}
