#ifndef MEDIO_H
#define MEDIO_H

#include <string>

/**
 * @brief Foto o vídeo guardado en disco. La BD solo guarda la ruta del archivo.
 *        idUsuario = -1 cuando lo generó la IA (sin usuario).
 * @author Ignacio Arenas Ramos
 */
class Medio {
private:
    int idEsp32;
    int idUsuario;          // -1 = lo generó la IA
    std::string tipo;       // "foto" | "video"
    std::string origen;     // "manual" | "ia"
    std::string ruta;

public:
    Medio(int idEsp32, int idUsuario, const std::string& tipo,
          const std::string& origen, const std::string& ruta);

    int getIdEsp32();
    int getIdUsuario();
    std::string getTipo();
    std::string getOrigen();
    std::string getRuta();
};

#endif
