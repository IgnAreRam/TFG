#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <cstdint>

/**
 * @brief Protocolo binario TCP de El Centinela.
 *        Cabecera fija de 5 bytes: [TIPO:1][LONGITUD:4 big-endian] + payload.
 *        El primer byte que envía un cliente decide si es ESP32 (0x10) o Flutter (0x30).
 * @author Ignacio Arenas Ramos
 */

// ── ESP32 → Servidor ──────────────────────────────────────────
#define MSG_ESP32_AUTH               0x10   // payload: texto "nombre:clave"
#define MSG_FRAME                    0x11   // payload: bytes JPEG
#define MSG_SERVO_OK                 0x12   // payload: vacío

// ── Servidor → ESP32 ──────────────────────────────────────────
#define MSG_AUTH_OK                  0x20   // payload: vacío
#define MSG_AUTH_FAIL                0x21   // payload: vacío
#define MSG_SERVO_CMD                0x22   // payload: pan(2)+tilt(2) big-endian
#define MSG_LED_CMD                  0x23   // payload: 1 byte (1=on, 0=off)

// ── Flutter → Servidor ────────────────────────────────────────
#define MSG_LOGIN                    0x30   // payload: texto "usuario:contraseña"
#define MSG_LISTAR_CAMARAS           0x31   // payload: vacío
#define MSG_SUSCRIBIR                0x32   // payload: id_camara(4)
#define MSG_SERVO_MANUAL             0x33   // payload: pan(2)+tilt(2)
#define MSG_LED_MANUAL               0x34   // payload: 1 byte
#define MSG_PEDIR_REGISTROS_IA       0x35   // payload: id_camara(4) (0=todas)
#define MSG_PEDIR_REGISTROS_USUARIO  0x36   // payload: vacío
#define MSG_CAPTURAR_FOTO            0x37   // payload: vacío (cámara suscrita)
#define MSG_INICIAR_GRABACION        0x38   // payload: vacío
#define MSG_PARAR_GRABACION          0x39   // payload: vacío
#define MSG_LISTAR_MEDIOS            0x3A   // payload: id_camara(4) (0=todas)
#define MSG_PEDIR_MEDIO              0x3B   // payload: id_medio(4)
#define MSG_PEDIR_LOGS               0x3C   // payload: vacío

// ── Servidor → Flutter ────────────────────────────────────────
#define MSG_LOGIN_OK                 0x40   // payload: id_usuario(4)
#define MSG_LOGIN_FAIL               0x41   // payload: vacío
#define MSG_LISTA_CAMARAS            0x42   // payload: texto "id;nombre;activa\n..."
#define MSG_SUSCRITO_OK              0x43   // payload: vacío
#define MSG_SUSCRITO_FAIL            0x44   // payload: vacío
#define MSG_FRAME_FLUTTER            0x45   // payload: bytes JPEG (vídeo en vivo)
#define MSG_REGISTROS_IA             0x46   // payload: texto "id_camara;fecha;tipo;confianza\n..."
#define MSG_REGISTROS_USUARIO        0x47   // payload: texto "usuario;fecha;accion;id_camara\n..."
#define MSG_LISTA_MEDIOS             0x48   // payload: texto "id;tipo;origen;fecha;id_camara\n..."
#define MSG_MEDIO_INICIO             0x49   // payload: tipo(1: 0=foto,1=video) + tamanoTotal(4)
#define MSG_MEDIO_TROZO              0x4A   // payload: bytes (un trozo del archivo)
#define MSG_MEDIO_FIN                0x4B   // payload: vacío
#define MSG_LOGS                     0x4C   // payload: texto (últimas líneas del log)

// Límite de payload para no reservar memoria enorme ante un cliente malicioso.
const uint32_t MAX_PAYLOAD = 300000;   // ~300 KB, suficiente para un JPEG QVGA

/**
 * Envía un mensaje completo (cabecera + payload). Devuelve false si el socket falla.
 */
bool enviarMensaje(int fd, uint8_t tipo, const unsigned char* payload, uint32_t longitud);

/**
 * Lee un mensaje completo. Reserva 'payload' con new[] (el llamante debe hacer delete[]).
 * Si longitud==0, payload queda en nullptr. Devuelve false si el cliente se desconecta.
 */
bool leerMensaje(int fd, uint8_t& tipo, unsigned char*& payload, uint32_t& longitud);

#endif
