#ifndef GESTORBD_H
#define GESTORBD_H

#include <string>
#include <pqxx/pqxx> 
#include <vector>

using namespace std;

// variables // que almaceno y funcionalidad principal
// pqxx::connection* conexion: Puntero a la conexión activa con PostgreSQL.
// string string_conexion: Datos de acceso (usuario, host, dbname, password).
// Funcionalidad principal: Centralizar la persistencia de datos del TFG. Gestiona tanto el hardware (sesiones, eventos, detecciones, multimedia) como el software (CRUD de clientes Flutter).

class GestorBD {
private:
    pqxx::connection* conexion;
    string string_conexion;

public:
    // metodos/funciones 
    // @param entrada 1 string con la cadena de conexión de PostgreSQL
    // @param devuelve Instancia de GestorBD
    // @funcion principal Constructor que inicializa la conexión con la base de datos.
    // @autor ignacio arenas ramos
    GestorBD(const string& conn_str);

    // metodos/funciones 
    // @param devuelve void
    // @funcion principal Destructor que cierra la conexión de forma segura.
    // @autor ignacio arenas ramos
    ~GestorBD();

    // ==========================================
    // 1. HARDWARE: CÁMARAS, SESIONES E IA
    // ==========================================

    // metodos/funciones 
    // @param entrada 1 id del dispositivo físico (int)
    // @param entrada 2 dirección IP desde la que se conecta el ESP32 (string)
    // @param devuelve ID de la sesión creada o -1 si falla (int)
    // @funcion principal Registra el inicio de una conexión de cámara en la tabla sesiones_camara.
    // @autor ignacio arenas ramos
    int registrarInicioSesion(int id_dispositivo, const string& ip_origen);

    // metodos/funciones 
    // @param entrada 1 ID de la sesión a cerrar (int)
    // @param entrada 2 Motivo del cierre (desconexion_normal, timeout, error_red) (string)
    // @param devuelve void
    // @funcion principal Actualiza la sesión para marcar su fin y el motivo de la desconexión.
    // @autor ignacio arenas ramos
    void cerrarSesion(int id_sesion, const string& motivo);

    // metodos/funciones 
    // @param entrada 1 ID de la sesión activa (int)
    // @param devuelve ID del evento de movimiento generado o -1 si falla (int)
    // @funcion principal Registra una ráfaga de movimiento detectada inicialmente por el ESP32.
    // @autor ignacio arenas ramos
    int registrarEventoMovimiento(int id_sesion);

    // metodos/funciones 
    // @param entrada 1 ID del evento de movimiento vinculado (int)
    // @param entrada 2 Clase de objeto (persona, perro, pajaro) (string)
    // @param entrada 3 Nivel de confianza 0.0 - 1.0 (float)
    // @param entrada 4 coord_x (int)
    // @param entrada 5 coord_y (int)
    // @param entrada 6 width (int)
    // @param entrada 7 height (int)
    // @param entrada 8 Indica si fue el objetivo fijado por el Auto-Lock (bool)
    // @param devuelve ID de la detección registrada o -1 si falla (int)
    // @funcion principal Guarda los resultados del análisis YOLO de un frame específico.
    // @autor ignacio arenas ramos
    int registrarDeteccion(int id_evento, const string& tipo, float confianza, int x, int y, int w, int h, bool lock);

    // metodos/funciones 
    // @param entrada 1 ID de la detección que originó la grabación (int)
    // @param entrada 2 Tipo de archivo ('clip' o 'foto') (string)
    // @param entrada 3 Ruta absoluta en el disco del servidor (string)
    // @param entrada 4 Duración en segundos, 0 si es foto (float)
    // @param entrada 5 Tamaño del archivo en bytes (long)
    // @param entrada 6 ID del usuario si es manual, o -1 si es automático (int)
    // @param devuelve void
    // @funcion principal Registra la ubicación y metadatos de un archivo de vídeo o imagen en la BD.
    // @autor ignacio arenas ramos
    void registrarGrabacion(int id_deteccion, const string& tipo, const string& ruta, float duracion, long tamanyo, int id_usuario = -1);


    // ==========================================
    // 2. CLIENTES: APP FLUTTER (CRUD USUARIOS)
    // ==========================================

    // metodos/funciones 
    // @param entrada 1 nombre del usuario (string)
    // @param entrada 2 contraseña ya encriptada/hasheada (string)
    // @param devuelve ID del nuevo usuario o -1 si falla (int)
    // @funcion principal Crea un nuevo usuario en la base de datos (Registro).
    // @autor ignacio arenas ramos
    int crearUsuario(const string& nombre, const string& password_hash);

    // metodos/funciones 
    // @param entrada 1 nombre del usuario (string)
    // @param entrada 2 contraseña encriptada enviada por la app (string)
    // @param devuelve ID del usuario si el login es correcto, -1 si falla (int)
    // @funcion principal Verifica si las credenciales coinciden en la base de datos (Login).
    // @autor ignacio arenas ramos
    int verificarLogin(const string& nombre, const string& password_hash);

    // metodos/funciones 
    // @param entrada 1 id del usuario a modificar (int)
    // @param entrada 2 nuevo nombre a establecer (string)
    // @param devuelve booleano indicando el éxito de la operación (bool)
    // @funcion principal Actualiza el nombre de un cliente existente.
    // @autor ignacio arenas ramos
    bool actualizarNombreUsuario(int id_usuario, const string& nuevo_nombre);

    // metodos/funciones 
    // @param entrada 1 id del usuario a borrar (int)
    // @param devuelve booleano indicando el éxito de la operación (bool)
    // @funcion principal Elimina un usuario del sistema de forma permanente.
    // @autor ignacio arenas ramos
    bool borrarUsuario(int id_usuario);
};

#endif