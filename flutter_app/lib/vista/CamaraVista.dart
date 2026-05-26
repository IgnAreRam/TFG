import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import '../controladora/ConexionServidor.dart';
import '../modelo/Camara.dart';

/// Pestaña "En vivo": vídeo de la cámara + cruceta de servos + flash + grabar + foto.
class CamaraVista extends StatefulWidget {
  final ConexionServidor conexion;
  final Camara? camara;
  const CamaraVista({super.key, required this.conexion, required this.camara});

  @override
  State<CamaraVista> createState() => _CamaraVistaState();
}

class _CamaraVistaState extends State<CamaraVista> {
  static const int paso = 5;   // grados por pulsación (pequeño para no provocar picos de corriente)

  Uint8List? _frame;
  StreamSubscription<Uint8List>? _subFrame;

  int _pan = 90;
  int _tilt = 90;
  bool _flash = false;
  bool _grabando = false;

  @override
  void initState() {
    super.initState();
    _subFrame = widget.conexion.onFrame.listen((jpeg) => setState(() => _frame = jpeg));
  }

  // Mueve el servo un paso pequeño y lo manda al servidor.
  void _mover(int dPan, int dTilt) {
    setState(() {
      _pan = (_pan + dPan).clamp(0, 180);
      _tilt = (_tilt + dTilt).clamp(0, 180);
    });
    widget.conexion.moverServo(_pan, _tilt);
  }

  void _recentrar() {
    setState(() { _pan = 90; _tilt = 90; });
    widget.conexion.moverServo(_pan, _tilt);
  }

  void _foto() {
    widget.conexion.capturarFoto();
    ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('Foto capturada')));
  }

  void _toggleGrabar() {
    setState(() => _grabando = !_grabando);
    if (_grabando) {
      widget.conexion.iniciarGrabacion();
    } else {
      widget.conexion.pararGrabacion();
    }
  }

  @override
  void dispose() {
    _subFrame?.cancel();
    super.dispose();
  }

  Widget _flecha(IconData icono, VoidCallback al) =>
      IconButton.filledTonal(iconSize: 34, icon: Icon(icono), onPressed: al);

  @override
  Widget build(BuildContext context) {
    if (widget.camara == null) {
      return const Center(child: Text('Selecciona una cámara en la pestaña "Cámaras".'));
    }

    return ListView(
      padding: const EdgeInsets.all(12),
      children: [
        // Vídeo: lo giramos 90° (RotatedBox) para verlo en horizontal.
        Container(
          height: 260,
          color: Colors.black,
          alignment: Alignment.center,
          child: _frame == null
              ? const Text('Esperando vídeo...', style: TextStyle(color: Colors.white))
              : RotatedBox(
                  quarterTurns: 1,
                  child: Image.memory(_frame!, gaplessPlayback: true, fit: BoxFit.contain),
                ),
        ),
        const SizedBox(height: 12),

        // CRUCETA de servos.
        Text('Pan $_pan°   ·   Tilt $_tilt°', textAlign: TextAlign.center),
        Column(
          children: [
            // Signos invertidos para que la dirección coincida con la cámara en landscape.
            _flecha(Icons.keyboard_arrow_up, () => _mover(0, -paso)),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                _flecha(Icons.keyboard_arrow_left, () => _mover(paso, 0)),
                IconButton.filled(
                    iconSize: 30,
                    icon: const Icon(Icons.center_focus_strong),
                    onPressed: _recentrar),
                _flecha(Icons.keyboard_arrow_right, () => _mover(-paso, 0)),
              ],
            ),
            _flecha(Icons.keyboard_arrow_down, () => _mover(0, paso)),
          ],
        ),
        const Divider(),

        // Controles: flash, grabar, foto.
        SwitchListTile(
          title: const Text('Flash (LED)'),
          secondary: const Icon(Icons.flash_on),
          value: _flash,
          onChanged: (v) { setState(() => _flash = v); widget.conexion.led(v); },
        ),
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            FilledButton.icon(
              onPressed: _toggleGrabar,
              icon: Icon(_grabando ? Icons.stop : Icons.fiber_manual_record),
              label: Text(_grabando ? 'Parar' : 'Grabar'),
              style: _grabando
                  ? FilledButton.styleFrom(backgroundColor: Colors.red)
                  : null,
            ),
            FilledButton.icon(onPressed: _foto, icon: const Icon(Icons.photo_camera), label: const Text('Foto')),
          ],
        ),
      ],
    );
  }
}
