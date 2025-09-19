#!/bin/bash

# Script de prueba principal para el sensor daemon
# Ejecuta todas las pruebas disponibles

set -e

echo "=== Ejecutando pruebas del Sensor Daemon ==="
echo

# Verificar que el binario existe
if [ ! -f "../build/sensor_daemon" ]; then
    echo "ERROR: El binario no existe. Ejecuta 'make' primero."
    exit 1
fi

echo "✓ Binario encontrado: ../build/sensor_daemon"
echo

# Ejecutar prueba de fallback
echo "1. Ejecutando prueba de fallback..."
./test-fallback.sh
echo

# Ejecutar prueba de SIGTERM
echo "2. Ejecutando prueba de SIGTERM..."
./test-sigterm.sh
echo

# Ejecutar prueba en foreground
echo "3. Ejecutando prueba en foreground..."
./run-foreground.sh
echo

echo "=== Todas las pruebas completadas ==="
