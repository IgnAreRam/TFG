#ifndef REGISTROIA_H
#define REGISTROIA_H

#include <string>

/**
 * @brief Detección hecha por la IA: qué se vio, en qué cámara y con qué confianza.
 *        La fecha la pone PostgreSQL automáticamente (DEFAULT now()).
 * @author Ignacio Arenas Ramos
 */
class RegistroIA {
private:
    int idEsp32;
    std::string tipo;
    double confianza;

public:
    RegistroIA(int idEsp32, const std::string& tipo, double confianza);

    int getIdEsp32();
    std::string getTipo();
    double getConfianza();
};

#endif
