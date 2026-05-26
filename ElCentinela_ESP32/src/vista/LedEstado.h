#ifndef LEDESTADO_H
#define LEDESTADO_H

/**
 * @brief Controla el LED de la placa (encender/apagar).
 *        Lo usa el servidor con MSG_LED_CMD (incluido el flash de bienvenida).
 * @author Ignacio Arenas Ramos
 */
class LedEstado {
public:
    void iniciar();
    void encender();
    void apagar();
};

#endif
