#ifndef ESP32_H
#define ESP32_H

#include <string>

/**
 * @brief Cámara ESP32-CAM registrada en el sistema.
 * @author Ignacio Arenas Ramos
 */
class Esp32 {
private:
    int id;
    std::string nombre;
    bool activa;

public:
    Esp32(int id, const std::string& nombre, bool activa);

    int getId();
    std::string getNombre();
    bool getActiva();
};

#endif
