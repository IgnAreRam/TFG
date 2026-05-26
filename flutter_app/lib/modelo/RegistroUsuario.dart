/// Acción de un operador, tal como la envía el servidor:
/// "usuario;fecha_hora;accion;id_camara".
class RegistroUsuario {
  final String usuario;
  final String fecha;
  final String accion;
  final String camara;   // "-" cuando la acción no tiene cámara (login)

  RegistroUsuario(this.usuario, this.fecha, this.accion, this.camara);

  factory RegistroUsuario.desdeLinea(String linea) {
    final p = linea.split(';');
    return RegistroUsuario(p[0], p[1], p[2], p[3]);
  }
}
