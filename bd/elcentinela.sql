-- ============================================================
--  EL CENTINELA — Base de datos PostgreSQL
--  Autor: Ignacio Arenas Ramos
-- ============================================================
--  Ejecutar:  psql -U ignacio -d elcentinela -f elcentinela.sql
-- ============================================================

-- Borrar tablas previas (en orden inverso por las claves ajenas)
DROP TABLE IF EXISTS medio;
DROP TABLE IF EXISTS registro_usuario;
DROP TABLE IF EXISTS registro_ia;
DROP TABLE IF EXISTS esp32;
DROP TABLE IF EXISTS usuario;

-- ------------------------------------------------------------
--  USUARIO — operador que usa la app Flutter
-- ------------------------------------------------------------
CREATE TABLE usuario (
    id          SERIAL       PRIMARY KEY,
    usuario     VARCHAR(50)  UNIQUE NOT NULL,   -- nombre de login
    contrasena  VARCHAR(100) NOT NULL           -- texto plano (decisión de simplicidad del TFG)
);

-- ------------------------------------------------------------
--  ESP32 — cámara registrada en el sistema
-- ------------------------------------------------------------
CREATE TABLE esp32 (
    id      SERIAL       PRIMARY KEY,
    nombre  VARCHAR(50)  UNIQUE NOT NULL,        -- identificador que envía en MSG_ESP32_AUTH
    clave   VARCHAR(100) NOT NULL,               -- texto plano
    activa  BOOLEAN      NOT NULL DEFAULT TRUE    -- dada de alta/habilitada (NO es "conectada ahora")
);

-- ------------------------------------------------------------
--  REGISTRO_IA — qué detectó la IA, cuándo y con qué confianza
-- ------------------------------------------------------------
CREATE TABLE registro_ia (
    id          SERIAL    PRIMARY KEY,
    id_esp32    INTEGER   NOT NULL REFERENCES esp32(id),
    tipo        VARCHAR(30) NOT NULL,            -- "persona", "perro", "gato"...
    confianza   REAL      NOT NULL,              -- 0.0 a 1.0
    fecha_hora  TIMESTAMP NOT NULL DEFAULT now()
);

-- ------------------------------------------------------------
--  REGISTRO_USUARIO — acción de un operador (login, acceso, servo, led)
--  id_esp32 es NULL cuando la acción no implica cámara (p. ej. login)
-- ------------------------------------------------------------
CREATE TABLE registro_usuario (
    id          SERIAL    PRIMARY KEY,
    id_usuario  INTEGER   NOT NULL REFERENCES usuario(id),
    id_esp32    INTEGER   REFERENCES esp32(id),  -- NULL permitido (login no tiene cámara)
    accion      VARCHAR(50) NOT NULL,            -- "login", "acceso_camara", "mover_servo", "led"
    fecha_hora  TIMESTAMP NOT NULL DEFAULT now()
);

-- ------------------------------------------------------------
--  MEDIO — foto o vídeo guardado en disco (la BD solo guarda la ruta)
--  origen: 'manual' (lo pidió un usuario) o 'ia' (clip automático al detectar)
--  id_usuario es NULL cuando lo generó la IA
-- ------------------------------------------------------------
CREATE TABLE medio (
    id          SERIAL    PRIMARY KEY,
    id_esp32    INTEGER   NOT NULL REFERENCES esp32(id),
    id_usuario  INTEGER   REFERENCES usuario(id),   -- NULL si lo generó la IA
    tipo        VARCHAR(10) NOT NULL,               -- 'foto' | 'video'
    origen      VARCHAR(10) NOT NULL,               -- 'manual' | 'ia'
    ruta        VARCHAR(255) NOT NULL,              -- ruta del archivo en disco
    fecha_hora  TIMESTAMP NOT NULL DEFAULT now()
);

-- ------------------------------------------------------------
--  Datos de ejemplo para la demo
-- ------------------------------------------------------------
INSERT INTO usuario (usuario, contrasena) VALUES ('ignacio', '1234');

INSERT INTO esp32 (nombre, clave, activa) VALUES
    ('cam1', '1234', TRUE),
    ('cam2', '1234', TRUE);
