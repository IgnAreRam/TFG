#include "ControlServos.h"
#include "../modelo/Configuracion.h"

void ControlServos::iniciar() {
    // La cámara usa el timer LEDC 0; damos a los servos los timers 1-3 para que
    // no se peleen por el mismo temporizador PWM (si no, la imagen sale corrupta).
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    pan.setPeriodHertz(50);    // los servos trabajan a 50 Hz
    tilt.setPeriodHertz(50);
    pan.attach(PIN_SERVO_PAN, 500, 2400);
    tilt.attach(PIN_SERVO_TILT, 500, 2400);

    mover(90, 90);   // empezar centrado
}

void ControlServos::mover(int anguloPan, int anguloTilt) {
    if (anguloPan < 0) anguloPan = 0;     if (anguloPan > 180) anguloPan = 180;
    if (anguloTilt < 0) anguloTilt = 0;   if (anguloTilt > 180) anguloTilt = 180;
    pan.write(anguloPan);
    tilt.write(anguloTilt);
}
