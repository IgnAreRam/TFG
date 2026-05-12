#include <iostream>
#include <thread>
#include "GestorControl.h"
#include "ListenerESP32.h"

using namespace std;

int main() {
    cout << "=== INICIANDO SERVIDOR TFG (V6) ===" << endl;

    // 1. Instanciamos el Gestor Central
    GestorControl gestorCentral;

    // 2. Inicializamos el socket de escucha en el puerto 8080 (por ejemplo)
    int puerto_control = 8080;
    int socket_maestro = inicializarSocketMaestro(puerto_control);

    if (socket_maestro < 0) {
        cerr << "[FATAL] No se pudo abrir el socket. Saliendo..." << endl;
        return -1;
    }

    // 3. Lanzamos la hebra global (Listener de Control) en segundo plano
    // Usamos std::thread para que el main no se quede bloqueado en el bucle infinito
    thread hebra_listener(hebraListenerESP32, socket_maestro, std::ref(gestorCentral));

    // 4. Detach para que la hebra corra por su cuenta de forma independiente
    hebra_listener.detach();

    // Aquí en el futuro inicializaremos la Hebra IA y el Listener de Flutter...
    // Por ahora, para que el main no se cierre, lo ponemos a dormir.
    while(true) {
        // El main se queda aquí como Watchdog general o simplemente esperando
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}