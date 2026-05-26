#include "RegistroUsuarioDAO.h"
#include "ConexionBD.h"

using namespace std;

void RegistroUsuarioDAO::insertar(ConexionBD* bd, RegistroUsuario& registro) {
    // Si idEsp32 es -1 mandamos NULL real a la columna (acción sin cámara, p.ej. login).
    if (registro.getIdEsp32() < 0) {
        bd->ejecutar(
            "INSERT INTO registro_usuario (id_usuario, id_esp32, accion) VALUES ($1, NULL, $2)",
            { to_string(registro.getIdUsuario()), registro.getAccion() });
    } else {
        bd->ejecutar(
            "INSERT INTO registro_usuario (id_usuario, id_esp32, accion) VALUES ($1, $2, $3)",
            { to_string(registro.getIdUsuario()),
              to_string(registro.getIdEsp32()),
              registro.getAccion() });
    }
}

string RegistroUsuarioDAO::listar(ConexionBD* bd) {
    PGresult* res = bd->consultar(
        "SELECT u.usuario, r.fecha_hora, r.accion, COALESCE(r.id_esp32::text, '-') "
        "FROM registro_usuario r JOIN usuario u ON u.id = r.id_usuario "
        "ORDER BY r.fecha_hora DESC LIMIT 100",
        {});

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
