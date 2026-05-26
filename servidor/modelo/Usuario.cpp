#include "Usuario.h"

using namespace std;

Usuario::Usuario(int id, const string& nombre) {
    this->id = id;
    this->nombre = nombre;
}

int Usuario::getId()             { return id; }
string Usuario::getNombre() { return nombre; }
