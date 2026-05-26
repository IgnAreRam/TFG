#include "GestorConexiones.h"
#include "HiloESP32.h"

using namespace std;

GestorConexiones::GestorConexiones() {
    pthread_mutex_init(&mutex, nullptr);
}

GestorConexiones::~GestorConexiones() {
    pthread_mutex_destroy(&mutex);
}

void GestorConexiones::registrar(HiloESP32* camara) {
    // ── SINCRONIZACIÓN: añadir a la lista compartida bajo mutex ──
    pthread_mutex_lock(&mutex);
    // Si ya había una cámara con el mismo id (reconexión con el socket viejo colgado),
    // la echamos: cortamos su socket para que su hilo termine y se autodestruya.
    for (size_t i = 0; i < camaras.size(); ) {

        if (camaras[i]->getId() == camara->getId()) {

            HiloESP32* viejo = camaras[i];
            camaras.erase(camaras.begin() + i);
            viejo->solicitarCierre();

        } else {
            i++;
        }
    }

    camaras.push_back(camara);
    pthread_mutex_unlock(&mutex);
}

void GestorConexiones::eliminar(HiloESP32* camara) {

    pthread_mutex_lock(&mutex);

    for (size_t i = 0; i < camaras.size(); i++) {

        if (camaras[i] == camara) { camaras.erase(camaras.begin() + i); break; }

    }

    pthread_mutex_unlock(&mutex);
}

HiloESP32* GestorConexiones::buscarPorId(int id) {

    HiloESP32* encontrada = nullptr;
    pthread_mutex_lock(&mutex);

    for (size_t i = 0; i < camaras.size(); i++) {

        if (camaras[i]->getId() == id) { encontrada = camaras[i]; break; }
    }

    pthread_mutex_unlock(&mutex);
    return encontrada;
}

bool GestorConexiones::existe(HiloESP32* camara) {

    bool esta = false;
    pthread_mutex_lock(&mutex);

    for (size_t i = 0; i < camaras.size(); i++) {

        if (camaras[i] == camara) { esta = true; break; }
    }
    pthread_mutex_unlock(&mutex);
    return esta;
}

string GestorConexiones::listar() {

    string texto;

    pthread_mutex_lock(&mutex);

    for (size_t i = 0; i < camaras.size(); i++) {
        
        texto += to_string(camaras[i]->getId()) + ";" +
                 camaras[i]->getNombre() + ";1\n";   // 1 = conectada ahora mismo
    }
    pthread_mutex_unlock(&mutex);
    return texto;
}
