#ifndef CONFIGURACION_H
#define CONFIGURACION_H

/**
 * @brief Constantes de configuración del ESP32-CAM.
 *        Cambia aquí el WiFi y la dirección de ngrok antes de subir el sketch.
 * @author Ignacio Arenas Ramos
 */

// --- WiFi ---
#define WIFI_SSID  "MiWiFi"
#define WIFI_PASS  "contrasena_wifi"

// --- Servidor (lo que devuelve "ngrok tcp 8080", p.ej. 0.tcp.ngrok.io:12345) ---
#define SERVIDOR_HOST    "0.tcp.ngrok.io"
#define SERVIDOR_PUERTO  12345

// --- Identidad de la cámara (debe existir en la tabla esp32 de la BD) ---
#define CAMARA_NOMBRE  "cam1"
#define CAMARA_CLAVE   "1234"

// --- Pines ---
// OJO: GPIO 12 es un "strapping pin" (voltaje de la flash al arrancar). Suele funcionar
// con un servo, pero si la placa no bootea tras conectarlo, mueve el pan a otro pin (p.ej. 14).
#define PIN_SERVO_PAN   12
#define PIN_SERVO_TILT  13
#define PIN_LED          4   // LED flash de la placa AI Thinker

#endif
