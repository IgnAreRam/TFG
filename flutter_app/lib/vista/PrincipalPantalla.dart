import 'package:flutter/material.dart';
import '../controladora/ConexionServidor.dart';
import '../modelo/Camara.dart';
import 'CamarasVista.dart';
import 'CamaraVista.dart';
import 'MediosVista.dart';
import 'RegistrosVista.dart';

/// Pantalla principal tras el login: barra de navegación con 3 pestañas.
///  0) Lista de cámaras ESP32.
///  1) Vídeo en vivo + cruceta + flash + grabar + foto.
///  2) Galería de medios (fotos/vídeos) + botón de logs.
class PrincipalPantalla extends StatefulWidget {
  final ConexionServidor conexion;
  final String usuario;
  const PrincipalPantalla({super.key, required this.conexion, required this.usuario});

  @override
  State<PrincipalPantalla> createState() => _PrincipalPantallaState();
}

class _PrincipalPantallaState extends State<PrincipalPantalla> {
  int _indice = 0;
  Camara? _camaraActiva;

  // La llama la lista cuando eliges una cámara: la activa y salta a la pestaña de vídeo.
  void _seleccionar(Camara camara) {
    setState(() {
      _camaraActiva = camara;
      _indice = 1;
    });
  }

  @override
  Widget build(BuildContext context) {
    final paginas = [
      CamarasVista(conexion: widget.conexion, onSeleccionar: _seleccionar),
      CamaraVista(conexion: widget.conexion, camara: _camaraActiva),
      MediosVista(conexion: widget.conexion),
      RegistrosVista(conexion: widget.conexion),
    ];
    final titulos = ['Cámaras', 'En vivo', 'Galería', 'Registros'];

    return Scaffold(
      appBar: AppBar(title: Text('${titulos[_indice]}  ·  ${widget.usuario}')),
      // IndexedStack mantiene vivas las 3 pestañas (el vídeo no se corta al cambiar).
      body: IndexedStack(index: _indice, children: paginas),
      bottomNavigationBar: NavigationBar(
        selectedIndex: _indice,
        onDestinationSelected: (i) => setState(() => _indice = i),
        destinations: const [
          NavigationDestination(icon: Icon(Icons.videocam), label: 'Cámaras'),
          NavigationDestination(icon: Icon(Icons.live_tv), label: 'En vivo'),
          NavigationDestination(icon: Icon(Icons.perm_media), label: 'Galería'),
          NavigationDestination(icon: Icon(Icons.history), label: 'Registros'),
        ],
      ),
    );
  }
}
