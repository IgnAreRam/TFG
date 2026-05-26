import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:video_player/video_player.dart';
import '../controladora/ConexionServidor.dart';
import '../modelo/Medio.dart';

/// Pestaña "Galería": lista las fotos y vídeos (de la IA y manuales), permite
/// verlos, y tiene un botón para ver los logs del servidor.
class MediosVista extends StatefulWidget {
  final ConexionServidor conexion;
  const MediosVista({super.key, required this.conexion});

  @override
  State<MediosVista> createState() => _MediosVistaState();
}

class _MediosVistaState extends State<MediosVista> {
  List<Medio> _medios = [];
  StreamSubscription<List<Medio>>? _subMedios;
  StreamSubscription<MedioDescargado>? _subDescarga;
  StreamSubscription<String>? _subLogs;

  @override
  void initState() {
    super.initState();
    _subMedios = widget.conexion.onMedios.listen((l) => setState(() => _medios = l));
    _subDescarga = widget.conexion.onMedioDescargado.listen(_verMedio);
    _subLogs = widget.conexion.onLogs.listen(_verLogs);
    widget.conexion.listarMedios(0);
  }

  void _verMedio(MedioDescargado m) {
    if (!mounted) return;
    if (m.esVideo) {
      showDialog(context: context, builder: (_) => _DialogoVideo(bytes: m.bytes));
    } else {
      showDialog(context: context, builder: (_) => Dialog(child: Image.memory(m.bytes)));
    }
  }

  void _verLogs(String texto) {
    if (!mounted) return;
    showDialog(context: context, builder: (_) => AlertDialog(
      title: const Text('Logs del servidor'),
      content: SizedBox(
        width: double.maxFinite,
        child: SingleChildScrollView(
          child: Text(texto.isEmpty ? '(vacío)' : texto,
              style: const TextStyle(fontFamily: 'monospace', fontSize: 11)),
        ),
      ),
      actions: [TextButton(onPressed: () => Navigator.pop(context), child: const Text('Cerrar'))],
    ));
  }

  @override
  void dispose() {
    _subMedios?.cancel();
    _subDescarga?.cancel();
    _subLogs?.cancel();
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
              FilledButton.tonalIcon(
                onPressed: () => widget.conexion.listarMedios(0),
                icon: const Icon(Icons.refresh),
                label: const Text('Refrescar'),
              ),
              const Spacer(),
              FilledButton.tonalIcon(
                onPressed: () => widget.conexion.pedirLogs(),
                icon: const Icon(Icons.terminal),
                label: const Text('Logs del server'),
              ),
            ],
          ),
        ),
        Expanded(
          child: _medios.isEmpty
              ? const Center(child: Text('Aún no hay fotos ni vídeos.'))
              : ListView(
                  children: _medios.map((m) => Card(
                        child: ListTile(
                          leading: Icon(m.esVideo ? Icons.movie : Icons.image, size: 30),
                          title: Text('${m.tipo}  ·  ${m.origen == 'ia' ? 'IA' : 'manual'}'),
                          subtitle: Text('cámara ${m.idCamara}  ·  ${m.fecha}'),
                          trailing: const Icon(Icons.play_circle_outline),
                          onTap: () {
                            ScaffoldMessenger.of(context).showSnackBar(
                                const SnackBar(content: Text('Descargando...'), duration: Duration(seconds: 1)));
                            widget.conexion.pedirMedio(m.id);
                          },
                        ),
                      )).toList(),
                ),
        ),
      ],
    );
  }
}

/// Reproductor de vídeo: guarda los bytes en un archivo temporal y los reproduce.
class _DialogoVideo extends StatefulWidget {
  final List<int> bytes;
  const _DialogoVideo({required this.bytes});

  @override
  State<_DialogoVideo> createState() => _DialogoVideoState();
}

class _DialogoVideoState extends State<_DialogoVideo> {
  VideoPlayerController? _controlador;

  @override
  void initState() {
    super.initState();
    _preparar();
  }

  Future<void> _preparar() async {
    final dir = await Directory.systemTemp.createTemp('medio');
    final archivo = File('${dir.path}/video.mp4');
    await archivo.writeAsBytes(widget.bytes);
    final c = VideoPlayerController.file(archivo);
    await c.initialize();
    await c.setLooping(true);
    await c.play();
    if (mounted) setState(() => _controlador = c);
  }

  @override
  void dispose() {
    _controlador?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Dialog(
      child: _controlador == null
          ? const SizedBox(height: 200, child: Center(child: CircularProgressIndicator()))
          : AspectRatio(
              aspectRatio: _controlador!.value.aspectRatio,
              child: VideoPlayer(_controlador!),
            ),
    );
  }
}
