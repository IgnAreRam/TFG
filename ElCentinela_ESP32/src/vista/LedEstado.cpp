#include "LedEstado.h"
#include <Arduino.h>
#include "../modelo/Configuracion.h"

void LedEstado::iniciar() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);   // empieza apagado
}

void LedEstado::encender() { digitalWrite(PIN_LED, HIGH); }
void LedEstado::apagar()   { digitalWrite(PIN_LED, LOW); }
