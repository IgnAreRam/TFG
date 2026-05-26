#include "Logger.h"
#include <pthread.h>
#include <cstdio>

using namespace std;

// ── SINCRONIZACIÓN: mutex para que dos hilos no mezclen sus líneas en consola ──
static pthread_mutex_t mutexLog = PTHREAD_MUTEX_INITIALIZER;

// Imprime el mensaje con su color y una etiqueta, todo bajo el mutex.
static void imprimir(const char* color, const char* etiqueta, const string& msg) {
    pthread_mutex_lock(&mutexLog);
    printf("%s[%s]\033[0m %s\n", color, etiqueta, msg.c_str());
    fflush(stdout);
    pthread_mutex_unlock(&mutexLog);
}

void Logger::info(const string& msg)    { imprimir("\033[37m", "INFO", msg); }
void Logger::ok(const string& msg)      { imprimir("\033[32m", "OK",   msg); }
void Logger::warning(const string& msg) { imprimir("\033[33m", "WARN", msg); }
void Logger::error(const string& msg)   { imprimir("\033[31m", "ERROR", msg); }
void Logger::debug(const string& msg)   { imprimir("\033[90m", "DEBUG", msg); }
