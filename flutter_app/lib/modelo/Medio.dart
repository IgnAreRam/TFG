import 'dart:typed_data';

/// Foto o vídeo, tal como lo lista el servidor: "id;tipo;origen;fecha;id_camara".
class Medio {
  final int id;
  final String tipo;     // "foto" | "video"
  final String origen;   // "manual" | "ia"
  final String fecha;
  final int idCamara;

  Medio(this.id, this.tipo, this.origen, this.fecha, this.idCamara);

  bool get esVideo => tipo == 'video';

  factory Medio.desdeLinea(String linea) {
    final p = linea.split(';');
    return Medio(int.parse(p[0]), p[1], p[2], p[3], int.parse(p[4]));
  }
}

/// Un medio ya descargado del servidor (bytes en memoria).
class MedioDescargado {
  final bool esVideo;   // true = mp4, false = jpg
  final Uint8List bytes;
  MedioDescargado(this.esVideo, this.bytes);
}
