#ifndef LOGGER_H
#define LOGGER_H

#include <string>

/**
 * @brief Registro de mensajes por consola con colores ANSI.
 *        Métodos estáticos: no hace falta instanciar nada.
 * @author Ignacio Arenas Ramos
 */
class Logger {
public:
    static void info(const std::string& msg);     // blanco
    static void ok(const std::string& msg);        // verde
    static void warning(const std::string& msg);   // amarillo
    static void error(const std::string& msg);     // rojo
    static void debug(const std::string& msg);     // gris
};

#endif
