import 'dart:async';
import 'package:flutter/material.dart';
import '../controladora/ConexionServidor.dart';
import '../modelo/RegistroIA.dart';
import '../modelo/RegistroUsuario.dart';

/// Pestaña "Registros": muestra las detecciones de la IA y las acciones de usuario.
/// Se piden al servidor (mensajes 0x35 / 0x36) y este responde con texto.
class RegistrosVista extends StatefulWidget {
  final ConexionServidor conexion;
  const RegistrosVista({super.key, required this.conexion});

  @override
  State<RegistrosVista> createState() => _RegistrosVistaState();
}

class _RegistrosVistaState extends State<RegistrosVista> {
  bool _mostrandoIA = true;
  List<RegistroIA> _ia = [];
  List<RegistroUsuario> _usr = [];
  StreamSubscription? _subIA;
  StreamSubscription? _subUsr;

  @override
  void initState() {
    super.initState();
    _subIA = widget.conexion.onRegistrosIA.listen((l) => setState(() => _ia = l));
    _subUsr = widget.conexion.onRegistrosUsuario.listen((l) => setState(() => _usr = l));
    widget.conexion.pedirRegistrosIA(0);   // 0 = todas las cámaras
  }

  void _cargar() {
    if (_mostrandoIA) {
      widget.conexion.pedirRegistrosIA(0);
    } else {
      widget.conexion.pedirRegistrosUsuario();
    }
  }

  @override
  void dispose() {
    _subIA?.cancel();
    _subUsr?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Padding(
          padding: const EdgeInsets.all(8),
          child: Row(
            children: [
              ChoiceChip(
                label: const Text('IA'),
                selected: _mostrandoIA,
                onSelected: (_) { setState(() => _mostrandoIA = true); widget.conexion.pedirRegistrosIA(0); },
              ),
              const SizedBox(width: 8),
              ChoiceChip(
                label: const Text('Acciones'),
                selected: !_mostrandoIA,
                onSelected: (_) { setState(() => _mostrandoIA = false); widget.conexion.pedirRegistrosUsuario(); },
              ),
              const Spacer(),
              IconButton(icon: const Icon(Icons.refresh), onPressed: _cargar),
            ],
          ),
        ),
        Expanded(child: _mostrandoIA ? _listaIA() : _listaUsuario()),
      ],
    );
  }

  Widget _listaIA() => _ia.isEmpty
      ? const Center(child: Text('Sin detecciones'))
      : ListView(
          children: _ia.map((r) => Card(
                child: ListTile(
                  leading: const Icon(Icons.smart_toy),
                  title: Text('${r.tipo}  ·  conf ${r.confianza}'),
                  subtitle: Text('cámara ${r.idCamara}  ·  ${r.fecha}'),
                ),
              )).toList(),
        );

  Widget _listaUsuario() => _usr.isEmpty
      ? const Center(child: Text('Sin acciones'))
      : ListView(
          children: _usr.map((r) => Card(
                child: ListTile(
                  leading: const Icon(Icons.person),
                  title: Text('${r.usuario}  ·  ${r.accion}'),
                  subtitle: Text('cámara ${r.camara}  ·  ${r.fecha}'),
                ),
              )).toList(),
        );
}
