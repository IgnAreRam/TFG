#include "ConexionBD.h"
#include "Logger.h"

using namespace std;

// Convierte el vector de strings al array de const char* que pide libpq.
static const char** construirParams(const vector<string>& params) {
    if (params.empty()) return nullptr;
    const char** valores = new const char*[params.size()];   // liberado tras la llamada
    for (size_t i = 0; i < params.size(); i++) valores[i] = params[i].c_str();
    return valores;
}

ConexionBD::ConexionBD(const string& cadenaConexion) {
    pthread_mutex_init(&mutex, nullptr);
    conexion = PQconnectdb(cadenaConexion.c_str());
    if (PQstatus(conexion) != CONNECTION_OK)
        Logger::error(string("BD: ") + PQerrorMessage(conexion));
    else
        Logger::ok("Conectado a PostgreSQL");
}

ConexionBD::~ConexionBD() {
    if (conexion) PQfinish(conexion);          // cierra la conexión a PostgreSQL
    pthread_mutex_destroy(&mutex);
}

bool ConexionBD::conectado() {
    return conexion && PQstatus(conexion) == CONNECTION_OK;
}

PGresult* ConexionBD::consultar(const string& sql, const vector<string>& params) {
    // ── SINCRONIZACIÓN: solo un hilo puede usar la conexión a la vez ──
    pthread_mutex_lock(&mutex);
    const char** valores = construirParams(params);
    PGresult* res = PQexecParams(conexion, sql.c_str(), params.size(),
                                 nullptr, valores, nullptr, nullptr, 0);
    delete[] valores;                          // solo el array de punteros, no las strings
    pthread_mutex_unlock(&mutex);
    return res;
}

bool ConexionBD::ejecutar(const string& sql, const vector<string>& params) {
    PGresult* res = consultar(sql, params);
    bool ok = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!ok) Logger::error(string("BD: ") + PQerrorMessage(conexion));
    PQclear(res);
    return ok;
}
