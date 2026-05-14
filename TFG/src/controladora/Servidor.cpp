#include "Servidor.h"
#include "ListenerESP32.h"
// #include "ListenerClientes.h" // Pronto lo crearemos

Servidor::Servidor(int p_camaras, int p_clientes) 
    : puerto_camaras(p_camaras), puerto_clientes(p_clientes), activo(false) {
    
    // 1. Conectamos a la BD usando la cadena de conexión de tu PC/Servidor
    gestor_bd = new GestorBD("dbname=tfg_db user=postgres password=root host=127.0.0.1");

    // 2. Instanciamos la IA pasándole todo lo que necesita para vivir (Cola, Servos y Base de Datos)
    ia = new ProcesadorIA(cola_video, gestor_hardware, *gestor_bd, "yolov4.weights", "yolov4.cfg");
}

void Servidor::ejecutar() {
    activo = true;
    cout << "--- INICIANDO SISTEMA DE SEGURIDAD TFG ---" << endl;

    // 1. Lanzamos la IA en su propia hebra para que se quede esperando fotogramas
    thread t_ia(&ProcesadorIA::iniciarBucleInferencia, ia);
    t_ia.detach();

    // 2. Lanzamos el Listener de Cámaras (ESP32) pasándole la cola para que la rellene
    thread t_red_hardware(hebraListenerESP32, puerto_camaras, std::ref(gestor_hardware), std::ref(cola_video));
    t_red_hardware.detach();

    // 3. Lanzamos el Listener de la App (Flutter) pasándole el control y la BD
    // thread t_red_app(hebraListenerClientes, puerto_clientes, std::ref(gestor_hardware), std::ref(*gestor_bd));
    // t_red_app.detach();

    // El hilo principal se queda manteniendo todo vivo
    while (activo) {
        gestor_hardware.revisarConexiones(45); 
        this_thread::sleep_for(chrono::seconds(5));
    }
}