#include "ServicioIA.h"
#include "Logger.h"
#include <unistd.h>
#include <cstring>
#include <cstdlib>

using namespace std;

ServicioIA::ServicioIA() {
    pipeEntrada = -1;
    salidaPython = nullptr;
    pidPython = -1;
    pthread_mutex_init(&mutex, nullptr);
}

ServicioIA::~ServicioIA() {
    if (salidaPython) fclose(salidaPython);
    if (pipeEntrada >= 0) close(pipeEntrada);   // cerrar el pipe hace que el Python termine
    pthread_mutex_destroy(&mutex);
}

// Lanza yolo_server.py como proceso hijo con dos tuberías (una de ida y otra de vuelta).
bool ServicioIA::iniciar(const string& script) {
    int pipeStdin[2];    // C++ escribe el frame -> stdin del Python
    int pipeStdout[2];   // Python escribe el JSON -> lo leemos
    if (pipe(pipeStdin) < 0 || pipe(pipeStdout) < 0) return false;

    // ── PROCESOS: fork crea el hijo que ejecutará Python ──
    pidPython = fork();
    if (pidPython < 0) return false;

    if (pidPython == 0) {
        // --- proceso HIJO (Python) ---
        dup2(pipeStdin[0],  STDIN_FILENO);    // su stdin = extremo de lectura del pipe de entrada
        dup2(pipeStdout[1], STDOUT_FILENO);   // su stdout = extremo de escritura del pipe de salida
        // cerrar los extremos que no usa el hijo
        close(pipeStdin[0]);  close(pipeStdin[1]);
        close(pipeStdout[0]); close(pipeStdout[1]);
        execlp("python3", "python3", script.c_str(), (char*)nullptr);
        _exit(1);   // solo se llega aquí si execlp falla
    }

    // --- proceso PADRE (servidor C++) ---
    close(pipeStdin[0]);    // el padre no lee de su propia entrada
    close(pipeStdout[1]);   // ni escribe en la salida del Python
    pipeEntrada = pipeStdin[1];
    salidaPython = fdopen(pipeStdout[0], "r");   // FILE* para usar fgets cómodamente
    Logger::ok("Proceso YOLO lanzado (pid " + to_string(pidPython) + ")");
    return true;
}

// Extrae un campo de texto del JSON: busca clave "x":" y copia hasta la siguiente comilla.
static string extraerTexto(const string& s, const string& clave) {
    size_t p = s.find(clave);
    if (p == string::npos) return "";
    p += clave.size();
    size_t fin = s.find('"', p);
    return s.substr(p, fin - p);
}

// Extrae un número del JSON: busca clave "x": y lee hasta la coma o el cierre.
static double extraerNumero(const string& s, const string& clave) {
    size_t p = s.find(clave);
    if (p == string::npos) return 0;
    return atof(s.c_str() + p + clave.size());
}

Deteccion ServicioIA::analizar(const unsigned char* jpeg, int tam) {
    Deteccion d;
    d.detectado = false;

    // ── SINCRONIZACIÓN: solo un hilo IA usa el proceso YOLO a la vez ──
    pthread_mutex_lock(&mutex);

    // IDA: enviar tamaño (4 bytes big-endian) + bytes JPEG por el pipe.
    unsigned char cab[4] = {
        (unsigned char)((tam >> 24) & 0xFF), (unsigned char)((tam >> 16) & 0xFF),
        (unsigned char)((tam >> 8)  & 0xFF), (unsigned char)( tam        & 0xFF) };
    if (write(pipeEntrada, cab, 4) == 4 && write(pipeEntrada, jpeg, tam) == tam) {
        // VUELTA: leer una línea de JSON del stdout del Python.
        char linea[512];
        if (fgets(linea, sizeof(linea), salidaPython)) {
            string s(linea);
            if (s.find("\"detectado\":true") != string::npos) {
                d.detectado = true;
                d.tipo = extraerTexto(s, "\"tipo\":\"");
                d.cx = (int)extraerNumero(s, "\"cx\":");
                d.cy = (int)extraerNumero(s, "\"cy\":");
                d.confianza = extraerNumero(s, "\"confianza\":");
            }
        }
    }

    pthread_mutex_unlock(&mutex);
    return d;
}
