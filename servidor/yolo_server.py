#!/usr/bin/env python3
"""
yolo_server.py - proceso hijo del servidor C++.

Lee frames JPEG por stdin y devuelve por stdout una linea JSON con la mejor
deteccion. La comunicacion es por tuberias (pipes), sin HTTP ni Flask.

IMPORTANTE: cada respuesta lleva flush=True. Sin flush, Python guarda la
salida en un buffer interno y el servidor C++ se queda esperando para siempre
(este fue el bug clasico en Ubuntu Server).
"""
import sys
import io
import json
from ultralytics import YOLO
from PIL import Image

# yolov8n ('nano'): el mas rapido en CPU -> seguimiento mas fluido (mas fps).
# Menos preciso que el 'small', se compensa con el umbral de confianza y buena luz.
model = YOLO('yolov8n.pt')

# Confianza minima para aceptar una deteccion (sube este valor si hay falsos
# positivos, bajalo si no detecta bien). 0.35 equilibra detectar sin demasiados falsos.
CONFIANZA_MIN = 0.25

# Clases de YOLO (COCO) que nos interesan -> nombre en espanol.
OBJETIVOS = {
    'person': 'persona', 'dog': 'perro', 'cat': 'gato',
    'bird': 'pajaro', 'horse': 'caballo', 'sheep': 'oveja',
    'cow': 'vaca', 'bear': 'oso',
}


def leer_exacto(n):
    """Lee exactamente n bytes de stdin; devuelve None si la tuberia se cierra."""
    datos = b''
    while len(datos) < n:
        trozo = sys.stdin.buffer.read(n - len(datos))
        if not trozo:
            return None
        datos += trozo
    return datos


def main():
    while True:
        # ── IDA: leer el tamano del frame (4 bytes big-endian) y luego los bytes JPEG ──
        cabecera = leer_exacto(4)
        if cabecera is None:
            break
        tam = int.from_bytes(cabecera, 'big')
        jpeg = leer_exacto(tam)
        if jpeg is None:
            break

        resultado = {'detectado': False}
        try:
            imagen = Image.open(io.BytesIO(jpeg))
            # ── YOLO: analizar el frame ──
            # imgsz=320: inferimos al tamaño real del frame (no a 640 por defecto),
            # es ~4x más rápido en CPU. conf filtra detecciones de baja confianza.
            salida = model(imagen, verbose=False, conf=CONFIANZA_MIN, imgsz=320)

            # Quedarnos con la deteccion de mayor confianza entre nuestros objetivos.
            mejor = None
            for r in salida:
                for caja in r.boxes:
                    nombre = model.names[int(caja.cls[0])]
                    if nombre not in OBJETIVOS:
                        continue
                    conf = float(caja.conf[0])
                    if mejor is None or conf > mejor['confianza']:
                        x1, y1, x2, y2 = caja.xyxy[0].tolist()
                        mejor = {
                            'detectado': True,
                            'tipo': OBJETIVOS[nombre],
                            'cx': int((x1 + x2) / 2),
                            'cy': int((y1 + y2) / 2),
                            'confianza': round(conf, 2),
                        }
            if mejor:
                resultado = mejor
        except Exception:
            resultado = {'detectado': False}

        # ── VUELTA: escribir el JSON compacto + salto de linea. flush OBLIGATORIO ──
        sys.stdout.write(json.dumps(resultado, separators=(',', ':')) + '\n')
        sys.stdout.flush()


if __name__ == '__main__':
    main()
