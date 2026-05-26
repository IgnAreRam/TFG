import 'package:flutter/material.dart';
import 'vista/LoginPantalla.dart';

void main() => runApp(const CentinelaApp());

class CentinelaApp extends StatelessWidget {
  const CentinelaApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'El Centinela',
      theme: ThemeData(colorSchemeSeed: Colors.indigo, useMaterial3: true),
      home: const LoginPantalla(),
    );
  }
}
