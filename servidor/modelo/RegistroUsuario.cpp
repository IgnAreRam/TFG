#include "RegistroUsuario.h"

using namespace std;

RegistroUsuario::RegistroUsuario(int idUsuario, int idEsp32, const string& accion) {
    this->idUsuario = idUsuario;
    this->idEsp32 = idEsp32;
    this->accion = accion;
}

int RegistroUsuario::getIdUsuario()    { return idUsuario; }
int RegistroUsuario::getIdEsp32()      { return idEsp32; }
string RegistroUsuario::getAccion(){ return accion; }
