#ifndef CONEXIONSERVIDOR_H
#define CONEXIONSERVIDOR_H

#include <WiFi.h>
#include <Arduino.h>

class ControlServos;
class LedEstado;

/**
 * @brief Conexión TCP con el servidor usando el protocolo binario de El Centinela
 *        (cabecera de 5 bytes: [TIPO:1][LONGITUD:4 big-endian] + payload).
 * @author Ignacio Arenas Ramos
 */
class ConexionServidor {
private:
    WiFiClient cliente;

    void enviarMensaje(uint8_t tipo, const uint8_t* payload, uint32_t longitud);
    bool leerExacto(uint8_t* buffer, uint32_t n, uint32_t timeoutMs);

public:
    void conectarWifi();
    bool conectar();
    bool autenticar();
    bool conectado();

    void enviarFrame(const uint8_t* datos, uint32_t longitud);
    void procesarComandos(ControlServos* servos, LedEstado* led);
};

#endif
