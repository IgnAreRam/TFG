#ifndef PROCESADORIA_H
#define PROCESADORIA_H

#include "ColaFrames.h"
#include "GestorControl.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <chrono>
#include <vector>

using namespace std;

// Estados del sistema
enum EstadoSeguimiento { BUSCANDO, SIGUIENDO, PERDIDO };

class ProcesadorIA {
private:
    ColaFrames& cola_entrada;
    GestorControl& gestor; // Para mandar órdenes a los servos
    cv::dnn::Net red_yolo;
    
    // Variables de control de estado
    EstadoSeguimiento estado_actual;
    int id_objetivo_actual;
    chrono::steady_clock::time_point ultima_vez_visto;
    
    // Posición actual de los servos (empezamos en el centro: 90, 90)
    int pos_x = 90;
    int pos_y = 90;

    void actualizarServos(int cam_id, cv::Rect box, int frame_width, int frame_height);
    void realizarBarrido(int cam_id);

    // --- LA NUEVA FUNCIÓN DE INFERENCIA ---
    // metodos/funciones 
    // @param entrada 1 Referencia a la imagen decodificada (cv::Mat&)
    // @param devuelve Un vector con los rectángulos (Bounding Boxes) de los objetos encontrados
    // @funcion principal Preprocesa la imagen, la pasa por la red YOLO y extrae las coordenadas de las detecciones.
    // @autor ignacio arenas ramos
    vector<cv::Rect> detectarObjetos(cv::Mat& imagen);

public:
    // metodos/funciones 
    // @param entrada 1 Cola segura de fotogramas
    // @param entrada 2 Gestor de control para enviar ordenes
    // @param entrada 3 Ruta archivo weights YOLO
    // @param entrada 4 Ruta archivo cfg YOLO
    // @param devuelve Instancia del objeto
    // @funcion principal Constructor. Inicializa el procesador, carga el modelo y prepara estados.
    // @autor ignacio arenas ramos
    ProcesadorIA(ColaFrames& cola, GestorControl& g, const string& pesos, const string& cfg);
    
    // metodos/funciones 
    // @param devuelve void
    // @funcion principal Bucle infinito de la hebra. Extrae frames, detecta objetos y gestiona el seguimiento autonomo.
    // @autor ignacio arenas ramos
    void iniciarBucleInferencia();
    
};

#endif