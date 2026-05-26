/// Detección de la IA, tal como la envía el servidor:
/// "id_camara;fecha_hora;tipo;confianza".
class RegistroIA {
  final String idCamara;
  final String fecha;
  final String tipo;
  final String confianza;

  RegistroIA(this.idCamara, this.fecha, this.tipo, this.confianza);

  factory RegistroIA.desdeLinea(String linea) {
    final p = linea.split(';');
    return RegistroIA(p[0], p[1], p[2], p[3]);
  }
}
