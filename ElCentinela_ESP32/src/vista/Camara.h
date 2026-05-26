#ifndef CAMARA_H
#define CAMARA_H

#include "esp_camera.h"

/**
 * @brief Maneja la cámara del ESP32-CAM (modelo AI Thinker).
 *        Captura frames JPEG a 320x240 (QVGA), el mismo tamaño que asume el
 *        servidor para centrar los servos.
 * @author Ignacio Arenas Ramos
 */
class Camara {
public:
    bool iniciar();
    camera_fb_t* capturar();          // devuelve el frame; hay que liberarlo después
    void liberar(camera_fb_t* fb);    // devuelve el buffer del frame a la cámara
};

#endif
