import 'dart:async';
import 'package:flutter/material.dart';
import '../controladora/ConexionServidor.dart';
import 'PrincipalPantalla.dart';

/// Pantalla inicial: pide host/puerto de ngrok y credenciales, conecta y loguea.
class LoginPantalla extends StatefulWidget {
  const LoginPantalla({super.key});

  @override
  State<LoginPantalla> createState() => _LoginPantallaState();
}

class _LoginPantallaState extends State<LoginPantalla> {
  final _host = TextEditingController(text: '0.tcp.ngrok.io');
  final _puerto = TextEditingController(text: '12345');
  final _usuario = TextEditingController(text: 'ignacio');
  final _contrasena = TextEditingController(text: '1234');

  final _conexion = ConexionServidor();
  StreamSubscription<bool>? _subLogin;
  bool _cargando = false;

  Future<void> _entrar() async {
    setState(() => _cargando = true);

    final ok = await _conexion.conectar(_host.text, int.tryParse(_puerto.text) ?? 0);
    if (!ok) {
      _mensaje('No se pudo conectar al servidor');
      setState(() => _cargando = false);
      return;
    }

    // Esperamos la respuesta del login por el stream.
    _subLogin = _conexion.onLogin.listen((correcto) {
      setState(() => _cargando = false);
      if (correcto) {
        Navigator.push(context, MaterialPageRoute(
          builder: (_) => PrincipalPantalla(conexion: _conexion, usuario: _usuario.text),
        ));
      } else {
        _mensaje('Usuario o contraseña incorrectos');
      }
    });

    _conexion.login(_usuario.text, _contrasena.text);
  }

  void _mensaje(String texto) {
    ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(texto)));
  }

  @override
  void dispose() {
    _subLogin?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('El Centinela')),
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: ListView(
          children: [
            TextField(controller: _host, decoration: const InputDecoration(labelText: 'Host (ngrok)')),
            TextField(controller: _puerto, decoration: const InputDecoration(labelText: 'Puerto'),
                keyboardType: TextInputType.number),
            TextField(controller: _usuario, decoration: const InputDecoration(labelText: 'Usuario')),
            TextField(controller: _contrasena, decoration: const InputDecoration(labelText: 'Contraseña'),
                obscureText: true),
            const SizedBox(height: 20),
            FilledButton(
              onPressed: _cargando ? null : _entrar,
              child: _cargando
                  ? const CircularProgressIndicator()
                  : const Text('Entrar'),
            ),
          ],
        ),
      ),
    );
  }
}
