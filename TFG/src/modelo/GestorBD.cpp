#include "GestorBD.h"
#include <iostream>

using namespace std;
using namespace pqxx; // Espacio de nombres de la librería oficial de PostgreSQL

GestorBD::GestorBD(const string& conn_str) : string_conexion(conn_str) {
    try {
        conexion = new connection(string_conexion);
        cout << "[BD] Conectado a PostgreSQL exitosamente." << endl;
    } catch (const exception& e) {
        cerr << "[FATAL BD] Error al conectar a la base de datos: " << e.what() << endl;
        conexion = nullptr;
    }
}

GestorBD::~GestorBD() {
    if (conexion && conexion->is_open()) {
        conexion->disconnect();
        delete conexion;
        cout << "[BD] Conexión cerrada limpiamente." << endl;
    }
}

// ==========================================
// 1. HARDWARE: CÁMARAS, SESIONES E IA
// ==========================================

int GestorBD::registrarInicioSesion(int id_dispositivo, const string& ip_origen) {
    if (!conexion) return -1;
    try {
        work txn(*conexion); // 'work' inicia una transacción segura
        // Usamos txn.quote() para evitar Inyecciones SQL (fundamental en un TFG)
        string sql = "INSERT INTO sesiones_camara (id_dispositivo, ip_origen, fecha_inicio, estado) "
                     "VALUES (" + txn.quote(id_dispositivo) + ", " + txn.quote(ip_origen) + ", NOW(), 'activa') "
                     "RETURNING id_sesion;";
        
        result r = txn.exec(sql);
        txn.commit(); // Confirmamos los cambios
        
        return r[0][0].as<int>(); // Devolvemos el ID generado
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al registrar sesión: " << e.what() << endl;
        return -1;
    }
}

void GestorBD::cerrarSesion(int id_sesion, const string& motivo) {
    if (!conexion) return;
    try {
        work txn(*conexion);
        string sql = "UPDATE sesiones_camara SET estado = 'cerrada', fecha_fin = NOW(), motivo_cierre = " 
                     + txn.quote(motivo) + " WHERE id_sesion = " + txn.quote(id_sesion) + ";";
        txn.exec0(sql); // exec0 porque no esperamos que devuelva nada
        txn.commit();
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al cerrar sesión: " << e.what() << endl;
    }
}

int GestorBD::registrarEventoMovimiento(int id_sesion) {
    if (!conexion) return -1;
    try {
        work txn(*conexion);
        string sql = "INSERT INTO eventos_movimiento (id_sesion, timestamp_inicio) "
                     "VALUES (" + txn.quote(id_sesion) + ", NOW()) RETURNING id_evento;";
        result r = txn.exec(sql);
        txn.commit();
        return r[0][0].as<int>();
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al registrar evento: " << e.what() << endl;
        return -1;
    }
}

int GestorBD::registrarDeteccion(int id_evento, const string& tipo, float confianza, int x, int y, int w, int h, bool lock) {
    if (!conexion) return -1;
    try {
        work txn(*conexion);
        string sql = "INSERT INTO detecciones (id_evento, tipo_objetivo, nivel_confianza, coord_x, coord_y, width, height, auto_lock) "
                     "VALUES (" + txn.quote(id_evento) + ", " + txn.quote(tipo) + ", " + txn.quote(confianza) + ", "
                     + txn.quote(x) + ", " + txn.quote(y) + ", " + txn.quote(w) + ", " + txn.quote(h) + ", " + txn.quote(lock) + ") "
                     "RETURNING id_deteccion;";
        result r = txn.exec(sql);
        txn.commit();
        return r[0][0].as<int>();
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al registrar detección: " << e.what() << endl;
        return -1;
    }
}

void GestorBD::registrarGrabacion(int id_deteccion, const string& tipo, const string& ruta, float duracion, long tamanyo, int id_usuario) {
    if (!conexion) return;
    try {
        work txn(*conexion);
        string id_usr_str = (id_usuario == -1) ? "NULL" : txn.quote(id_usuario);
        
        string sql = "INSERT INTO multimedia (id_deteccion, tipo_archivo, ruta_archivo, duracion_segundos, tamanyo_bytes, id_usuario_manual) "
                     "VALUES (" + txn.quote(id_deteccion) + ", " + txn.quote(tipo) + ", " + txn.quote(ruta) + ", " 
                     + txn.quote(duracion) + ", " + txn.quote(tamanyo) + ", " + id_usr_str + ");";
        txn.exec0(sql);
        txn.commit();
        cout << "[BD] Archivo multimedia guardado en BD: " << ruta << endl;
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al registrar multimedia: " << e.what() << endl;
    }
}

// ==========================================
// 2. CLIENTES: APP FLUTTER (CRUD USUARIOS)
// ==========================================

int GestorBD::crearUsuario(const string& nombre, const string& password_hash) {
    if (!conexion) return -1;
    try {
        work txn(*conexion);
        string sql = "INSERT INTO usuarios (nombre_usuario, password_hash, fecha_registro) "
                     "VALUES (" + txn.quote(nombre) + ", " + txn.quote(password_hash) + ", NOW()) "
                     "RETURNING id_usuario;";
        result r = txn.exec(sql);
        txn.commit();
        return r[0][0].as<int>();
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al crear usuario (¿Nombre duplicado?): " << e.what() << endl;
        return -1; // Suele fallar si el nombre ya existe (restricción UNIQUE)
    }
}

int GestorBD::verificarLogin(const string& nombre, const string& password_hash) {
    if (!conexion) return -1;
    try {
        work txn(*conexion);
        string sql = "SELECT id_usuario FROM usuarios "
                     "WHERE nombre_usuario = " + txn.quote(nombre) + " "
                     "AND password_hash = " + txn.quote(password_hash) + ";";
        result r = txn.exec(sql);
        
        if (!r.empty()) {
            return r[0][0].as<int>(); // Login exitoso
        }
        return -1; // Credenciales incorrectas
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al verificar login: " << e.what() << endl;
        return -1;
    }
}

bool GestorBD::actualizarNombreUsuario(int id_usuario, const string& nuevo_nombre) {
    if (!conexion) return false;
    try {
        work txn(*conexion);
        string sql = "UPDATE usuarios SET nombre_usuario = " + txn.quote(nuevo_nombre) + 
                     " WHERE id_usuario = " + txn.quote(id_usuario) + ";";
        result r = txn.exec(sql);
        txn.commit();
        return r.affected_rows() > 0; // True si modificó alguna fila
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al actualizar usuario: " << e.what() << endl;
        return false;
    }
}

bool GestorBD::borrarUsuario(int id_usuario) {
    if (!conexion) return false;
    try {
        work txn(*conexion);
        string sql = "DELETE FROM usuarios WHERE id_usuario = " + txn.quote(id_usuario) + ";";
        result r = txn.exec(sql);
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const exception& e) {
        cerr << "[ERROR BD] Fallo al borrar usuario: " << e.what() << endl;
        return false;
    }
}