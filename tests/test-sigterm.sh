#!/bin/bash

# Script para probar el manejo de SIGTERM
# Verifica que el daemon se detenga correctamente al recibir SIGTERM

set -e

echo "=== Prueba de Manejo de SIGTERM ==="

# Verificar que el binario existe
if [ ! -f "../build/sensor_daemon" ]; then
    echo "ERROR: El binario no existe. Ejecuta 'make' primero."
    exit 1
fi

# Limpiar logs anteriores
rm -f /tmp/sensor.log /var/tmp/sensor.log

echo "1. Iniciando daemon en background..."
../build/sensor_daemon 1 &
DAEMON_PID=$!

echo "Daemon iniciado con PID: $DAEMON_PID"

# Esperar un poco para que genere algunos logs
sleep 2

echo "2. Verificando que el daemon está corriendo..."
if kill -0 $DAEMON_PID 2>/dev/null; then
    echo "✓ Daemon está corriendo"
else
    echo "✗ Daemon no está corriendo"
    exit 1
fi

echo "3. Enviando SIGTERM al daemon..."
kill -TERM $DAEMON_PID

# Esperar a que termine
sleep 1

echo "4. Verificando que el daemon se detuvo correctamente..."
if kill -0 $DAEMON_PID 2>/dev/null; then
    echo "✗ Daemon aún está corriendo después de SIGTERM"
    kill -KILL $DAEMON_PID 2>/dev/null || true
    exit 1
else
    echo "✓ Daemon se detuvo correctamente"
fi

echo "5. Verificando logs generados..."
if [ -f "/tmp/sensor.log" ]; then
    echo "✓ Log encontrado en /tmp/sensor.log"
    echo "Contenido:"
    cat /tmp/sensor.log
    echo "Número de líneas: $(wc -l < /tmp/sensor.log)"
elif [ -f "/var/tmp/sensor.log" ]; then
    echo "✓ Log encontrado en /var/tmp/sensor.log"
    echo "Contenido:"
    cat /var/tmp/sensor.log
    echo "Número de líneas: $(wc -l < /var/tmp/sensor.log)"
else
    echo "✗ No se encontró ningún log"
fi

echo
echo "=== Prueba de SIGTERM completada ==="
