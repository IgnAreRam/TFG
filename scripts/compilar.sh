#!/bin/bash
# ============================================================
#  Compila el servidor C++ de El Centinela.
#  Uso:  ./compilar.sh
# ============================================================
set -e   # si algo falla, paramos

# Carpeta del servidor (relativa a este script, así funciona en cualquier ruta).
DIR="$(cd "$(dirname "$0")/../servidor" && pwd)"
cd "$DIR"

mkdir -p build
cd build
cmake ..
make -j"$(nproc)"

echo ""
echo "Compilado: $DIR/build/elcentinela_server"
