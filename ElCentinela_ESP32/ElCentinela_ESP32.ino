/**
 * @brief El Centinela - firmware del ESP32-CAM.
 *        setup(): WiFi -> cámara -> servos -> LED -> conectar -> autenticar.
 *        loop():  capturar JPEG -> enviar -> atender comandos (servo / led).
 * @author Ignacio Arenas Ramos
 */
#include "src/modelo/Configuracion.h"
#include "src/vista/Camara.h"
#include "src/vista/LedEstado.h"
#include "src/controladora/ControlServos.h"
#include "src/controladora/ConexionServidor.h"

Camara camara;
LedEstado led;
ControlServos servos;
ConexionServidor conexion;

void setup() {
    Serial.begin(115200);

    led.iniciar();
    servos.iniciar();
    if (!camara.iniciar()) {
        Serial.println("ERROR al iniciar la cámara");
    }

    conexion.conectarWifi();
    conexion.conectar();
    conexion.autenticar();
}

void loop() {
    // Si se cae la conexión, reconectar y volver a autenticar.
    if (!conexion.conectado()) {
        Serial.println("Reconectando...");
        if (conexion.conectar()) conexion.autenticar();
        delay(1000);
        return;
    }

    // 1) Capturar un frame y enviarlo.
    camera_fb_t* fb = camara.capturar();
    if (fb) {
        conexion.enviarFrame(fb->buf, fb->len);
        camara.liberar(fb);   // devolver el buffer a la cámara siempre
    }

    // 2) Atender comandos del servidor (mover servo / encender led).
    conexion.procesarComandos(&servos, &led);

    delay(50);   // ~20 frames por segundo
}
