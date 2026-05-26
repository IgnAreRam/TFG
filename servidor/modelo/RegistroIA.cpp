#include "RegistroIA.h"

using namespace std;

RegistroIA::RegistroIA(int idEsp32, const string& tipo, double confianza) {
    this->idEsp32 = idEsp32;
    this->tipo = tipo;
    this->confianza = confianza;
}

int RegistroIA::getIdEsp32()     { return idEsp32; }
string RegistroIA::getTipo(){ return tipo; }
double RegistroIA::getConfianza(){ return confianza; }
