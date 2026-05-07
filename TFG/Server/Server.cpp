#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
// #include <opencv2/opencv.hpp> // Lo descomentaremos cuando compilemos con OpenCV

using namespace std;

// variables //que almaceno y funcionalidad principal
// Estructura Data Transfer Object para los frames capturados.
// Almacena el origen del vídeo (id_camara), el frame de imagen y una marca de tiempo.
struct FrameData {
    int id_camara;
    // cv::Mat imagen; // Aquí irá el frame decodificado de OpenCV
    long long timestamp;
};

// variables //que almaceno y funcionalidad principal
// Monitor Concurrente (Cola) para la sincronización entre VideoESP (Productor) e IA (Consumidor).
// Almacena objetos FrameData y gestiona el límite de latencia mediante descartes (Drop oldest).
class ColaEntrada {
private:
    queue<FrameData> cola;
    mutex mtx;
    condition_variable cv;
    int limite_frames = 5; // Umbral de seguridad para mantener los 12 FPS sin lag

public:
    // metodos/funciones
    // @param entrada 1 el parametro de entrada es un objeto FrameData generado por la hebra VideoESP
    // @param devuelve un booleano indicando si el frame entró limpiamente (true) o si forzó un descarte (false)
    // @funcion principal este metodo sirve para inyectar un frame a la cola bloqueando el mutex. Si la cola está llena por retraso de la IA, descarta el frame más antiguo garantizando que la IA siempre vea el tiempo real.
    // @autor ignacio arenas ramos
    bool pushFrame(FrameData frame) {
        lock_guard<mutex> lock(mtx);
        bool hubo_descarte = false;
        
        if (cola.size() >= limite_frames) {
            cola.pop(); // Política LIFO adaptada: descartamos el más viejo, nos quedamos el nuevo
            hubo_descarte = true;
        }
        
        cola.push(frame);
        cv.notify_one(); // Despierta a la hebra IA
        return !hubo_descarte;
    }

    // metodos/funciones
    // @param devuelve un objeto FrameData extraído de forma segura para ser procesado
    // @funcion principal este metodo bloquea la hebra consumidora (IA) dejándola dormida (0 consumo CPU) hasta que VideoESP inyecte un nuevo frame. Una vez disponible, lo extrae y lo devuelve.
    // @autor ignacio arenas ramos
    FrameData popFrame() {
        unique_lock<mutex> lock(mtx);
        // Si la cola está vacía, la hebra espera pasivamente
        cv.wait(lock, [this]() { return !cola.empty(); });
        
        FrameData frame = cola.front();
        cola.pop();
        return frame;
    }
};