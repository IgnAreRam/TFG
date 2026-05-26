#ifndef CONEXIONBD_H
#define CONEXIONBD_H

#include <string>
#include <vector>
#include <pthread.h>
#include <libpq-fe.h>

/**
 * @brief Conexión única a PostgreSQL.
 *        libpq NO es thread-safe sobre la misma conexión, así que todos los
 *        accesos pasan por un mutex interno. Se usa siempre con parámetros
 *        ($1, $2...) para evitar inyección SQL.
 * @author Ignacio Arenas Ramos
 */
class ConexionBD {
private:
    PGconn* conexion;
    pthread_mutex_t mutex;   // serializa el acceso a la conexión

public:
    ConexionBD(const std::string& cadenaConexion);
    ~ConexionBD();

    bool conectado();

    // Consulta con parámetros (SELECT). Devuelve el PGresult; el llamante hace PQclear.
    PGresult* consultar(const std::string& sql, const std::vector<std::string>& params);

    // INSERT/UPDATE con parámetros. Devuelve true si fue bien.
    bool ejecutar(const std::string& sql, const std::vector<std::string>& params);
};

#endif
