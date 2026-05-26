import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

import '../modelo/Camara.dart';
import '../modelo/Medio.dart';
import '../modelo/RegistroIA.dart';
import '../modelo/RegistroUsuario.dart';

// --- Tipos del protocolo (mismos valores hex que el servidor C++) ---
const int msgLogin                  = 0x30;
const int msgListarCamaras          = 0x31;
const int msgSuscribir              = 0x32;
const int msgServoManual            = 0x33;
const int msgLedManual              = 0x34;
const int msgPedirRegistrosIA       = 0x35;
const int msgPedirRegistrosUsuario  = 0x36;
const int msgCapturarFoto           = 0x37;
const int msgIniciarGrabacion       = 0x38;
const int msgPararGrabacion         = 0x39;
const int msgListarMedios           = 0x3A;
const int msgPedirMedio             = 0x3B;
const int msgPedirLogs              = 0x3C;

const int msgLoginOk          = 0x40;
const int msgLoginFail        = 0x41;
const int msgListaCamaras     = 0x42;
const int msgSuscritoOk       = 0x43;
const int msgSuscritoFail     = 0x44;
const int msgFrameFlutter     = 0x45;
const int msgRegistrosIA      = 0x46;
const int msgRegistrosUsuario = 0x47;
const int msgListaMedios      = 0x48;
const int msgMedioInicio      = 0x49;
const int msgMedioTrozo       = 0x4A;
const int msgMedioFin         = 0x4B;
const int msgLogs             = 0x4C;

/// Conexión TCP con el servidor.
///
/// NOTA: Socket de dart:io es asíncrono y NO bloquea el hilo de UI, por eso no
/// se usa Isolate. El NetworkOnMainThreadException es de Android nativo (Java).
class ConexionServidor {
  Socket? _socket;
  final List<int> _buffer = [];

  // Recepción de un medio por trozos.
  bool _medioEsVideo = false;
  final BytesBuilder _medioBuffer = BytesBuilder();

  final _ctrlLogin     = StreamController<bool>.broadcast();
  final _ctrlCamaras   = StreamController<List<Camara>>.broadcast();
  final _ctrlSuscribir = StreamController<bool>.broadcast();
  final _ctrlFrame     = StreamController<Uint8List>.broadcast();
  final _ctrlMedios    = StreamController<List<Medio>>.broadcast();
  final _ctrlDescarga  = StreamController<MedioDescargado>.broadcast();
  final _ctrlLogs      = StreamController<String>.broadcast();
  final _ctrlRegIA     = StreamController<List<RegistroIA>>.broadcast();
  final _ctrlRegUsr    = StreamController<List<RegistroUsuario>>.broadcast();

  Stream<bool> get onLogin            => _ctrlLogin.stream;
  Stream<List<Camara>> get onCamaras  => _ctrlCamaras.stream;
  Stream<bool> get onSuscribir        => _ctrlSuscribir.stream;
  Stream<Uint8List> get onFrame       => _ctrlFrame.stream;
  Stream<List<Medio>> get onMedios    => _ctrlMedios.stream;
  Stream<MedioDescargado> get onMedioDescargado => _ctrlDescarga.stream;
  Stream<String> get onLogs           => _ctrlLogs.stream;
  Stream<List<RegistroIA>> get onRegistrosIA => _ctrlRegIA.stream;
  Stream<List<RegistroUsuario>> get onRegistrosUsuario => _ctrlRegUsr.stream;

  // ── CONEXIÓN ──────────────────────────────────────────────
  Future<bool> conectar(String host, int puerto) async {
    try {
      _socket = await Socket.connect(host, puerto, timeout: const Duration(seconds: 5));
      _socket!.listen(_alRecibir, onDone: cerrar, onError: (_) => cerrar());
      return true;
    } catch (e) {
      return false;
    }
  }

  void cerrar() {
    _socket?.destroy();
    _socket = null;
  }

  // ── RECEPCIÓN: extraer mensajes completos del flujo de bytes ──
  void _alRecibir(Uint8List datos) {
    _buffer.addAll(datos);
    while (_buffer.length >= 5) {
      final tipo = _buffer[0];
      final longitud = (_buffer[1] << 24) | (_buffer[2] << 16) | (_buffer[3] << 8) | _buffer[4];
      if (_buffer.length < 5 + longitud) break;
      final payload = Uint8List.fromList(_buffer.sublist(5, 5 + longitud));
      _buffer.removeRange(0, 5 + longitud);
      _manejar(tipo, payload);
    }
  }

  void _manejar(int tipo, Uint8List payload) {
    switch (tipo) {
      case msgLoginOk:      _ctrlLogin.add(true); break;
      case msgLoginFail:    _ctrlLogin.add(false); break;
      case msgSuscritoOk:   _ctrlSuscribir.add(true); break;
      case msgSuscritoFail: _ctrlSuscribir.add(false); break;
      case msgFrameFlutter: _ctrlFrame.add(payload); break;
      case msgListaCamaras:
        _ctrlCamaras.add(_parsear(payload, (l) => Camara.desdeLinea(l)));
        break;
      case msgListaMedios:
        _ctrlMedios.add(_parsear(payload, (l) => Medio.desdeLinea(l)));
        break;
      case msgLogs:
        _ctrlLogs.add(utf8.decode(payload, allowMalformed: true));
        break;
      case msgRegistrosIA:
        _ctrlRegIA.add(_parsear(payload, (l) => RegistroIA.desdeLinea(l)));
        break;
      case msgRegistrosUsuario:
        _ctrlRegUsr.add(_parsear(payload, (l) => RegistroUsuario.desdeLinea(l)));
        break;
      // --- Descarga de un medio por trozos ---
      case msgMedioInicio:
        _medioEsVideo = payload.isNotEmpty && payload[0] == 1;
        _medioBuffer.clear();
        break;
      case msgMedioTrozo:
        _medioBuffer.add(payload);
        break;
      case msgMedioFin:
        _ctrlDescarga.add(MedioDescargado(_medioEsVideo, _medioBuffer.toBytes()));
        _medioBuffer.clear();
        break;
    }
  }

  List<T> _parsear<T>(Uint8List payload, T Function(String) crear) {
    return utf8.decode(payload, allowMalformed: true)
        .split('\n')
        .where((l) => l.trim().isNotEmpty)
        .map(crear)
        .toList();
  }

  // ── ENVÍO ─────────────────────────────────────────────────
  void _enviar(int tipo, List<int> payload) {
    if (_socket == null) return;
    final len = payload.length;
    _socket!.add([tipo, (len >> 24) & 0xFF, (len >> 16) & 0xFF, (len >> 8) & 0xFF, len & 0xFF]);
    if (len > 0) _socket!.add(payload);
  }

  List<int> _u16(int v) => [(v >> 8) & 0xFF, v & 0xFF];
  List<int> _u32(int v) => [(v >> 24) & 0xFF, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF];

  void login(String usuario, String contrasena) => _enviar(msgLogin, utf8.encode('$usuario:$contrasena'));
  void listarCamaras() => _enviar(msgListarCamaras, []);
  void suscribir(int idCamara) => _enviar(msgSuscribir, _u32(idCamara));
  void moverServo(int pan, int tilt) => _enviar(msgServoManual, [..._u16(pan), ..._u16(tilt)]);
  void led(bool encendido) => _enviar(msgLedManual, [encendido ? 1 : 0]);

  void capturarFoto() => _enviar(msgCapturarFoto, []);
  void iniciarGrabacion() => _enviar(msgIniciarGrabacion, []);
  void pararGrabacion() => _enviar(msgPararGrabacion, []);
  void listarMedios(int idCamara) => _enviar(msgListarMedios, _u32(idCamara));
  void pedirMedio(int idMedio) => _enviar(msgPedirMedio, _u32(idMedio));
  void pedirLogs() => _enviar(msgPedirLogs, []);
  void pedirRegistrosIA(int idCamara) => _enviar(msgPedirRegistrosIA, _u32(idCamara));
  void pedirRegistrosUsuario() => _enviar(msgPedirRegistrosUsuario, []);
}
