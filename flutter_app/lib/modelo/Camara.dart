/// Cámara ESP32 tal como la lista el servidor: "id;nombre;activa".
class Camara {
  final int id;
  final String nombre;
  final bool activa;

  Camara(this.id, this.nombre, this.activa);

  factory Camara.desdeLinea(String linea) {
    final partes = linea.split(';');
    return Camara(int.parse(partes[0]), partes[1], partes[2] == '1');
  }
}
