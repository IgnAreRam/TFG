#include "GestorControl.h"

void GestorControl::agregarCamara(CamaraESP* nueva_camara) {
    lock_guard<mutex> lock(mtx_lista); // Bloquea el acceso mientras dura esta función
    lista_camaras[nueva_camara->getId()] = nueva_camara;
    cout << "[DB] Cámara " << nueva_camara->getId() << " registrada en el Gestor." << endl;
}

void GestorControl::procesarPing(int id_buscado) {
    lock_guard<mutex> lock(mtx_lista);
    if (lista_camaras.find(id_buscado) != lista_camaras.end()) {
        lista_camaras[id_buscado]->registrarLatido();
        cout << "[ALIVE] Latido recibido de cámara: " << id_buscado << endl;
    }
}

void GestorControl::revisarConexiones(int timeout_seg) {
    lock_guard<mutex> lock(mtx_lista);
    time_t ahora = time(nullptr);

    for (auto const& [id, camara] : lista_camaras) {
        if (camara->isOnline() && (ahora - camara->getUltimoPing() > timeout_seg)) {
            camara->setOffline();
            cout << "[WARNING] Cámara " << id << " ha pasado a estado OFFLINE." << endl;
            // Aquí en el futuro llamaremos a la Hebra DB para actualizar PostgreSQL
        }
    }
}