#include "ConexionServidor.h"
#include "ControlServos.h"
#include "../vista/LedEstado.h"
#include "../modelo/Configuracion.h"

// Tipos del protocolo que usa el ESP32 (mismos valores que el servidor).
#define MSG_ESP32_AUTH  0x10
#define MSG_FRAME       0x11
#define MSG_SERVO_OK    0x12
#define MSG_AUTH_OK     0x20
#define MSG_SERVO_CMD   0x22
#define MSG_LED_CMD     0x23

void ConexionServidor::conectarWifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" OK");
}

bool ConexionServidor::conectar() {
    return cliente.connect(SERVIDOR_HOST, SERVIDOR_PUERTO);
}

bool ConexionServidor::conectado() {
    return cliente.connected();
}

// Envía cabecera (5 bytes) + payload.
void ConexionServidor::enviarMensaje(uint8_t tipo, const uint8_t* payload, uint32_t longitud) {
    uint8_t cabecera[5];
    cabecera[0] = tipo;
    cabecera[1] = (longitud >> 24) & 0xFF;   // big-endian
    cabecera[2] = (longitud >> 16) & 0xFF;
    cabecera[3] = (longitud >> 8)  & 0xFF;
    cabecera[4] =  longitud        & 0xFF;
    cliente.write(cabecera, 5);
    if (longitud > 0) cliente.write(payload, longitud);
}

// Lee exactamente n bytes con un tiempo máximo de espera.
bool ConexionServidor::leerExacto(uint8_t* buffer, uint32_t n, uint32_t timeoutMs) {
    uint32_t leidos = 0;
    uint32_t inicio = millis();
    while (leidos < n) {
        if (cliente.available()) {
            int r = cliente.read(buffer + leidos, n - leidos);
            if (r > 0) leidos += r;
        } else {
            if (millis() - inicio > timeoutMs) return false;
            delay(1);
        }
    }
    return true;
}

bool ConexionServidor::autenticar() {
    // payload = "nombre:clave"
    String credenciales = String(CAMARA_NOMBRE) + ":" + String(CAMARA_CLAVE);
    enviarMensaje(MSG_ESP32_AUTH, (const uint8_t*)credenciales.c_str(), credenciales.length());

    // La respuesta es solo cabecera (payload vacío): leemos 5 bytes.
    uint8_t cabecera[5];
    if (!leerExacto(cabecera, 5, 5000)) return false;
    bool ok = (cabecera[0] == MSG_AUTH_OK);
    Serial.println(ok ? "Autenticado" : "Auth rechazada");
    return ok;
}

void ConexionServidor::enviarFrame(const uint8_t* datos, uint32_t longitud) {
    enviarMensaje(MSG_FRAME, datos, longitud);
}

// Procesa los comandos que el servidor haya enviado (servo / led).
void ConexionServidor::procesarComandos(ControlServos* servos, LedEstado* led) {
    while (cliente.available() >= 5) {
        uint8_t cabecera[5];
        if (!leerExacto(cabecera, 5, 1000)) return;

        uint8_t tipo = cabecera[0];
        uint32_t longitud = ((uint32_t)cabecera[1] << 24) | ((uint32_t)cabecera[2] << 16) |
                            ((uint32_t)cabecera[3] << 8)  |  (uint32_t)cabecera[4];

        uint8_t payload[8];
        if (longitud > sizeof(payload)) return;   // comando inesperado, salimos
        if (longitud > 0 && !leerExacto(payload, longitud, 1000)) return;

        if (tipo == MSG_SERVO_CMD && longitud == 4) {
            int pan  = (payload[0] << 8) | payload[1];
            int tilt = (payload[2] << 8) | payload[3];
            servos->mover(pan, tilt);
            enviarMensaje(MSG_SERVO_OK, nullptr, 0);   // confirmamos que movimos
        } else if (tipo == MSG_LED_CMD && longitud == 1) {
            if (payload[0] == 1) led->encender();
            else                 led->apagar();
        }
    }
}
