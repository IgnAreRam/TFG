#!/bin/bash
# ============================================================
#  Arranca El Centinela: ngrok (túnel) + servidor.
#  Mata las instancias anteriores si las hubiera.
#  El servidor lanza yolo_server.py internamente (pipes): NO se arranca aquí.
#  El authtoken de ngrok se configura una vez con:
#      ngrok config add-authtoken <token>
#  (NOTA: los túneles TCP de ngrok requieren verificar la cuenta en
#   dashboard.ngrok.com; si no, ngrok falla y solo sirve la IP local.)
#  Uso:  ./arrancar.sh
# ============================================================
pkill -f elcentinela_server 2>/dev/null
pkill -f "ngrok tcp" 2>/dev/null
sleep 1

DIR="$(cd "$(dirname "$0")/../servidor" && pwd)"
LOG="$(cd "$(dirname "$0")/.." && pwd)"

# Levantar ngrok en segundo plano (no bloquea; si falla seguimos con IP local).
setsid nohup ngrok tcp 8080 --log=stdout > "$LOG/ngrok.log" 2>&1 < /dev/null &
sleep 5
echo "Dirección pública de ngrok (si la cuenta está verificada):"
curl -s http://127.0.0.1:4040/api/tunnels 2>/dev/null \
    | grep -o 'tcp://[^"]*' | head -1 || echo "  (ngrok no disponible; usa la IP local 192.168.18.173:8080)"

# config.txt y yolo_server.py están en ../ (carpeta servidor/).
cd "$DIR/build"
./elcentinela_server ../config.txt
