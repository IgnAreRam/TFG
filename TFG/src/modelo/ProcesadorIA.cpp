#include "ProcesadorIA.h"

vector<cv::Rect> ProcesadorIA::detectarObjetos(cv::Mat& imagen) {
    vector<cv::Rect> cajas_detectadas;

    if (imagen.empty()) return cajas_detectadas;

    // 1. Preprocesamiento: Convertimos la imagen a "Blob" para YOLO
    cv::Mat blob;
    cv::dnn::blobFromImage(imagen, blob, 1/255.0, cv::Size(416, 416), cv::Scalar(0,0,0), true, false);

    // 2. Alimentamos la red neuronal
    red_yolo.setInput(blob);

    // 3. INFERENCIA REAL: Ejecutamos la red
    vector<cv::Mat> salidas_red;
    red_yolo.forward(salidas_red, red_yolo.getUnconnectedOutLayersNames());

    // 4. Procesamos los resultados de YOLO
    float umbral_confianza = 0.5; // Solo aceptamos detecciones con más del 50% de seguridad

    for (size_t i = 0; i < salidas_red.size(); ++i) {
        float* datos = (float*)salidas_red[i].data;
        for (int j = 0; j < salidas_red[i].rows; ++j, datos += salidas_red[i].cols) {
            
            // Extraemos las puntuaciones de las clases (Persona, Coche, etc.)
            cv::Mat puntuaciones = salidas_red[i].row(j).colRange(5, salidas_red[i].cols);
            cv::Point id_clase;
            double confianza;
            cv::minMaxLoc(puntuaciones, 0, &confianza, 0, &id_clase);

            // Supongamos que la clase "0" en YOLO es "Persona"
            if (confianza > umbral_confianza && id_clase.x == 0) {
                // Calculamos las coordenadas reales en base al tamaño de la imagen original
                int centro_x = (int)(datos[0] * imagen.cols);
                int centro_y = (int)(datos[1] * imagen.rows);
                int ancho = (int)(datos[2] * imagen.cols);
                int alto = (int)(datos[3] * imagen.rows);
                
                int x_esquina = centro_x - ancho / 2;
                int y_esquina = centro_y - alto / 2;

                // Guardamos el rectángulo
                cajas_detectadas.push_back(cv::Rect(x_esquina, y_esquina, ancho, alto));
            }
        }
    }

    // Nota: En producción real aquí se aplica NMS (Non-Maximum Suppression) 
    // para borrar cajas duplicadas sobre la misma persona, pero para empezar, esto es perfecto.

    return cajas_detectadas;
}

// @funcion principal Calcula el error entre el centro del objeto y el centro de la imagen 
// y ajusta los servos para compensarlo.
void ProcesadorIA::actualizarServos(int cam_id, cv::Rect box, int frame_width, int frame_height) {
    // 1. Calculamos el centro del objeto detectado
    int obj_centro_x = box.x + (box.width / 2);
    int obj_centro_y = box.y + (box.height / 2);

    // 2. Calculamos el centro de la pantalla
    int centro_pantalla_x = frame_width / 2;
    int centro_pantalla_y = frame_height / 2;

    // 3. Calculamos la diferencia (Error)
    int error_x = obj_centro_x - centro_pantalla_x;
    int error_y = obj_centro_y - centro_pantalla_y;

    // 4. Umbral de tolerancia (si está cerca del centro, no movemos para evitar vibraciones)
    int tolerancia = 30; 

    if (abs(error_x) > tolerancia) {
        // Si el error es positivo, movemos el servo un poco (ajuste fino de 2 grados)
        pos_x += (error_x > 0) ? -2 : 2; 
    }
    if (abs(error_y) > tolerancia) {
        pos_y += (error_y > 0) ? -2 : 2;
    }

    // Limitamos los servos entre 0 y 180 grados
    pos_x = std::max(0, std::min(180, pos_x));
    pos_y = std::max(0, std::min(180, pos_y));

    // 5. Enviamos la orden real al ESP32 a través del gestor
    gestor.moverServo(cam_id, "X", pos_x);
    gestor.moverServo(cam_id, "Y", pos_y);
}

// metodos/funciones 
// @param entrada 1 id de la camara a mover (int)
// @param devuelve void
// @funcion principal Mueve el servo en el eje X de lado a lado para buscar objetivos. Si termina y no hay nada, manda a dormir al ESP32.
// @autor ignacio arenas ramos
void ProcesadorIA::realizarBarrido(int cam_id) {
    cout << "[IA] Iniciando barrido en cámara " << cam_id << "..." << endl;

    // Hacemos un barrido simple de izquierda (30º) a derecha (150º) saltando de 10 en 10 grados
    for (int grados_x = 30; grados_x <= 150; grados_x += 10) {
        
        // 1. Movemos la cámara
        gestor.moverServo(cam_id, "X", grados_x);
        
        // 2. Le damos tiempo físico al servo para moverse y a la cámara para capturar un nuevo frame
        this_thread::sleep_for(chrono::milliseconds(300));
        
        // 3. Revisamos la cola a ver si en este nuevo ángulo vemos algo
        if (!cola_entrada.empty()) {
            FrameData frame_actual = cola_entrada.extraer();
            cv::Mat imagen = cv::imdecode(frame_actual.datos_imagen, cv::IMREAD_COLOR);
            
            vector<cv::Rect> detecciones = detectarObjetos(imagen); 
            
            if (!detecciones.empty()) {
                cout << "[IA] ¡Objetivo encontrado durante el barrido a " << grados_x << " grados!" << endl;
                estado_actual = SIGUIENDO; // Cambiamos de estado para que el bucle principal retome el control
                return; // Cortamos el barrido
            }
        }
    }

    // 4. Si el bucle termina, significa que hemos mirado por toda la habitación y no hay nadie
    cout << "[IA] Barrido terminado. Falsa alarma o el objetivo se fue." << endl;
    estado_actual = BUSCANDO;
    
    // Centramos la cámara de nuevo
    gestor.moverServo(cam_id, "X", 90);
    gestor.moverServo(cam_id, "Y", 90);
    
    // TODO: Mandar comando al ESP32 para que corte el streaming y vuelva a la dif de píxeles
    gestor.enviarComando(cam_id, "STOP_STREAM"); 
}

void ProcesadorIA::iniciarBucleInferencia() {
    while (true) {
        FrameData frame_crudo = cola_entrada.extraer();
        cv::Mat imagen = cv::imdecode(frame_crudo.datos_imagen, cv::IMREAD_COLOR);
        
        // Ejecutamos detección (simplificado para el ejemplo)
        // Supongamos que 'detecciones' contiene los rectángulos de las personas encontradas
        vector<cv::Rect> detecciones = detectarObjetos(imagen); 

        if (!detecciones.empty()) {
            // HEMOS ENCONTRADO ALGO
            estado_actual = SIGUIENDO;
            ultima_vez_visto = chrono::steady_clock::now();
            
            // Seguimos al primer objetivo que aparezca en la lista
            actualizarServos(frame_crudo.id_camara, detecciones[0], imagen.cols, imagen.rows);
        } else {
            // NO HAY NADIE EN PANTALLA
            auto ahora = chrono::steady_clock::now();
            auto tiempo_perdido = chrono::duration_cast<chrono::seconds>(ahora - ultima_vez_visto).count();

            if (estado_actual == SIGUIENDO && tiempo_perdido > 5) {
                // Si han pasado más de 5 segundos sin verlo, lo damos por PERDIDO
                estado_actual = BUSCANDO;
                cout << "[IA] Objetivo perdido. Volviendo a centro y barriendo..." << endl;
                
                // Ponemos servos en medio (90, 90)
                pos_x = 90; pos_y = 90;
                gestor.moverServo(frame_crudo.id_camara, "X", 90);
                gestor.moverServo(frame_crudo.id_camara, "Y", 90);
            }
            
            if (estado_actual == BUSCANDO) {
                realizarBarrido(frame_crudo.id_camara);
            }
        }
    }
}


