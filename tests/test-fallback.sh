#!/bin/bash

# Script para probar el mecanismo de fallback del logger
# Simula que /tmp no es escribible y verifica que use /var/tmp

set -e

echo "=== Prueba de Fallback del Logger ==="

# Verificar que el binario existe
if [ ! -f "../build/sensor_daemon" ]; then
    echo "ERROR: El binario no existe. Ejecuta 'make' primero."
    exit 1
fi

# Limpiar logs anteriores
rm -f /tmp/sensor.log /var/tmp/sensor.log

echo "1. Probando escritura normal en /tmp..."
../build/sensor_daemon 1 &
DAEMON_PID=$!
sleep 3
kill $DAEMON_PID 2>/dev/null || true
wait $DAEMON_PID 2>/dev/null || true

if [ -f "/tmp/sensor.log" ]; then
    echo "✓ Log creado en /tmp/sensor.log"
    echo "Contenido:"
    cat /tmp/sensor.log
else
    echo "✗ No se pudo crear log en /tmp"
fi

echo
echo "2. Probando fallback a /var/tmp..."
# Hacer /tmp no escribible temporalmente
chmod 000 /tmp 2>/dev/null || echo "No se pudo hacer /tmp no escribible (permisos insuficientes)"

../build/sensor_daemon 1 &
DAEMON_PID=$!
sleep 3
kill $DAEMON_PID 2>/dev/null || true
wait $DAEMON_PID 2>/dev/null || true

# Restaurar permisos de /tmp
chmod 755 /tmp 2>/dev/null || true

if [ -f "/var/tmp/sensor.log" ]; then
    echo "✓ Log creado en /var/tmp/sensor.log (fallback funcionó)"
    echo "Contenido:"
    cat /var/tmp/sensor.log
else
    echo "✗ Fallback no funcionó"
fi

echo
echo "=== Prueba de Fallback completada ==="
