#include "Medio.h"

using namespace std;

Medio::Medio(int idEsp32, int idUsuario, const string& tipo,
             const string& origen, const string& ruta) {
    this->idEsp32 = idEsp32;
    this->idUsuario = idUsuario;
    this->tipo = tipo;
    this->origen = origen;
    this->ruta = ruta;
}

int Medio::getIdEsp32()       { return idEsp32; }
int Medio::getIdUsuario()     { return idUsuario; }
string Medio::getTipo()  { return tipo; }
string Medio::getOrigen(){ return origen; }
string Medio::getRuta()  { return ruta; }
