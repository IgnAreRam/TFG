#!/bin/bash
# ============================================================
#  Instalación de El Centinela en la VM Ubuntu Server.
#  Se ejecuta UNA SOLA VEZ (primera puesta en marcha).
#  Uso:  ./instalar.sh
# ============================================================
set -e

echo ">> Instalando dependencias del sistema..."
sudo apt update
sudo apt install -y build-essential cmake libpq-dev postgresql python3 python3-pip

echo ">> Instalando dependencias de Python (YOLO)..."
pip3 install --user ultralytics pillow

echo ">> Creando usuario y base de datos en PostgreSQL..."
# El '|| true' evita que falle si ya existen de una ejecución anterior.
sudo -u postgres psql -c "CREATE USER ignacio WITH PASSWORD '1234';" || true
sudo -u postgres psql -c "CREATE DATABASE elcentinela OWNER ignacio;" || true

echo ">> Cargando el esquema (tablas + datos de ejemplo)..."
DIR="$(cd "$(dirname "$0")/.." && pwd)"
PGPASSWORD=1234 psql -h 127.0.0.1 -U ignacio -d elcentinela -f "$DIR/bd/elcentinela.sql"

echo ""
echo "Instalación terminada. Ahora:"
echo "  1) ./compilar.sh"
echo "  2) ngrok tcp 8080      (en otra terminal)"
echo "  3) ./arrancar.sh"
