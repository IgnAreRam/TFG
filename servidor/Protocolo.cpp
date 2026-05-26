#include "Protocolo.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

using namespace std;

// Lee exactamente 'n' bytes del socket (recv puede devolver menos de lo pedido).
static bool leerExacto(int fd, unsigned char* buffer, uint32_t n) {
    uint32_t leidos = 0;
    while (leidos < n) {
        ssize_t r = recv(fd, buffer + leidos, n - leidos, 0);
        if (r <= 0) return false;   // 0 = cerrado, <0 = error
        leidos += r;
    }
    return true;
}

// Envía exactamente 'n' bytes al socket (send puede enviar menos de lo pedido).
static bool enviarTodo(int fd, const unsigned char* buffer, uint32_t n) {
    uint32_t enviados = 0;
    while (enviados < n) {
        ssize_t e = send(fd, buffer + enviados, n - enviados, 0);
        if (e <= 0) return false;
        enviados += e;
    }
    return true;
}

bool enviarMensaje(int fd, uint8_t tipo, const unsigned char* payload, uint32_t longitud) {
    unsigned char cabecera[5];
    cabecera[0] = tipo;
    cabecera[1] = (longitud >> 24) & 0xFF;   // big-endian: byte más significativo primero
    cabecera[2] = (longitud >> 16) & 0xFF;
    cabecera[3] = (longitud >> 8)  & 0xFF;
    cabecera[4] =  longitud        & 0xFF;

    if (!enviarTodo(fd, cabecera, 5)) return false;
    if (longitud > 0 && !enviarTodo(fd, payload, longitud)) return false;
    return true;
}

bool leerMensaje(int fd, uint8_t& tipo, unsigned char*& payload, uint32_t& longitud) {
    unsigned char cabecera[5];
    if (!leerExacto(fd, cabecera, 5)) return false;

    tipo = cabecera[0];
    longitud = ((uint32_t)cabecera[1] << 24) | ((uint32_t)cabecera[2] << 16) |
               ((uint32_t)cabecera[3] << 8)  |  (uint32_t)cabecera[4];

    if (longitud > MAX_PAYLOAD) return false;   // protección contra tamaños absurdos

    payload = nullptr;
    if (longitud == 0) return true;

    payload = new unsigned char[longitud];      // lo libera el llamante con delete[]
    if (!leerExacto(fd, payload, longitud)) {
        delete[] payload;                        // si falla la lectura liberamos aquí mismo
        payload = nullptr;
        return false;
    }
    return true;
}
