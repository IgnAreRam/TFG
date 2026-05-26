#include "Esp32.h"

using namespace std;

Esp32::Esp32(int id, const string& nombre, bool activa) {
    this->id = id;
    this->nombre = nombre;
    this->activa = activa;
}

int Esp32::getId()             { return id; }
string Esp32::getNombre() { return nombre; }
bool Esp32::getActiva()        { return activa; }
