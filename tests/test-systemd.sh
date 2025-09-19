#!/bin/bash

# Script para probar el comportamiento del servicio systemd
# Simula la ejecución como lo haría systemd sin necesidad de instalarlo

set -e

echo "=== Prueba de Comportamiento Systemd ==="

# Verificar que el binario existe
if [ ! -f "../build/sensor_daemon" ]; then
    echo "ERROR: El binario no existe. Ejecuta 'make' primero."
    exit 1
fi

# Limpiar logs anteriores
rm -f /tmp/sensor.log /var/tmp/sensor.log

echo "1. Verificando archivo de servicio systemd..."
if [ -f "../systemd/assignment-sensor.service" ]; then
    echo "✓ Archivo de servicio encontrado"
    echo "Contenido:"
    cat ../systemd/assignment-sensor.service
else
    echo "✗ Archivo de servicio no encontrado"
    exit 1
fi

echo
echo "2. Simulando ejecución como usuario 'nobody'..."
echo "   (En un sistema real, systemd ejecutaría esto como usuario nobody)"

# Crear un directorio temporal para simular el entorno de nobody
TEMP_DIR=$(mktemp -d)
chmod 755 "$TEMP_DIR"

# Simular la ejecución del daemon como lo haría systemd
echo "   Ejecutando: /usr/local/bin/sensor_daemon 5"
echo "   (Usando el binario local en su lugar)"

../build/sensor_daemon 5 &
DAEMON_PID=$!

echo "   Daemon iniciado con PID: $DAEMON_PID"

# Esperar un poco para que genere logs
sleep 6

echo "3. Verificando que el daemon está generando logs..."
if [ -f "/tmp/sensor.log" ]; then
    echo "✓ Log encontrado en /tmp/sensor.log"
    echo "Últimas 3 líneas:"
    tail -3 /tmp/sensor.log
elif [ -f "/var/tmp/sensor.log" ]; then
    echo "✓ Log encontrado en /var/tmp/sensor.log"
    echo "Últimas 3 líneas:"
    tail -3 /var/tmp/sensor.log
else
    echo "✗ No se encontró ningún log"
fi

echo
echo "4. Probando restart on failure..."
echo "   Enviando SIGTERM para simular fallo..."
kill -TERM $DAEMON_PID
sleep 1

if kill -0 $DAEMON_PID 2>/dev/null; then
    echo "✗ Daemon no se detuvo con SIGTERM"
    kill -KILL $DAEMON_PID 2>/dev/null || true
else
    echo "✓ Daemon se detuvo correctamente con SIGTERM"
fi

echo
echo "5. Verificando formato de logs..."
if [ -f "/tmp/sensor.log" ]; then
    LOG_FILE="/tmp/sensor.log"
elif [ -f "/var/tmp/sensor.log" ]; then
    LOG_FILE="/var/tmp/sensor.log"
else
    echo "✗ No hay logs para verificar formato"
    exit 1
fi

echo "Verificando formato de timestamp ISO 8601..."
if grep -q "^[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}T[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}Z [0-9]\+$" "$LOG_FILE"; then
    echo "✓ Formato de timestamp correcto (ISO 8601)"
else
    echo "✗ Formato de timestamp incorrecto"
fi

# Limpiar
rm -rf "$TEMP_DIR"

echo
echo "=== Prueba de Systemd completada ==="
echo
echo "Para instalar el servicio real, ejecuta:"
echo "  sudo make install"
echo "  sudo systemctl enable --now assignment-sensor.service"
