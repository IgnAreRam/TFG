#ifndef CONTROLSERVOS_H
#define CONTROLSERVOS_H

#include <ESP32Servo.h>

/**
 * @brief Controla los dos servos de la cámara (pan = horizontal, tilt = vertical).
 * @author Ignacio Arenas Ramos
 */
class ControlServos {
private:
    Servo pan;
    Servo tilt;

public:
    void iniciar();
    void mover(int anguloPan, int anguloTilt);   // ángulos 0-180
};

#endif
