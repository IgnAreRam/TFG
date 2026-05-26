import 'dart:async';
import 'package:flutter/material.dart';
import '../controladora/ConexionServidor.dart';
import '../modelo/Camara.dart';

/// Pestaña "Cámaras": lista las cámaras conectadas. Al tocar una, te suscribes
/// y (vía onSeleccionar) la app salta a la pestaña de vídeo en vivo.
class CamarasVista extends StatefulWidget {
  final ConexionServidor conexion;
  final void Function(Camara) onSeleccionar;
  const CamarasVista({super.key, required this.conexion, required this.onSeleccionar});

  @override
  State<CamarasVista> createState() => _CamarasVistaState();
}

class _CamarasVistaState extends State<CamarasVista> {
  List<Camara> _camaras = [];
  StreamSubscription<List<Camara>>? _subCamaras;
  StreamSubscription<bool>? _subSuscribir;

  @override
  void initState() {
    super.initState();
    _subCamaras = widget.conexion.onCamaras.listen((lista) => setState(() => _camaras = lista));
    widget.conexion.listarCamaras();
  }

  void _abrir(Camara camara) {
    _subSuscribir?.cancel();
    _subSuscribir = widget.conexion.onSuscribir.listen((ok) {
      if (ok) {
        widget.onSeleccionar(camara);
      } else {
        ScaffoldMessenger.of(context)
            .showSnackBar(const SnackBar(content: Text('No se pudo acceder a la cámara')));
      }
    });
    widget.conexion.suscribir(camara.id);
  }

  @override
  void dispose() {
    _subCamaras?.cancel();
    _subSuscribir?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return RefreshIndicator(
      onRefresh: () async => widget.conexion.listarCamaras(),
      child: _camaras.isEmpty
          ? ListView(children: const [
              SizedBox(height: 200),
              Center(child: Text('No hay cámaras conectadas.\nDesliza hacia abajo para refrescar.',
                  textAlign: TextAlign.center)),
            ])
          : ListView(
              children: _camaras.map((c) => Card(
                    child: ListTile(
                      leading: const Icon(Icons.videocam, size: 32),
                      title: Text(c.nombre),
                      subtitle: Text('id ${c.id}  ·  conectada'),
                      trailing: const Icon(Icons.chevron_right),
                      onTap: () => _abrir(c),
                    ),
                  )).toList(),
            ),
    );
  }
}
