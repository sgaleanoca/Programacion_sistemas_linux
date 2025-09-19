#!/bin/bash

# Script para ejecutar el daemon en foreground para pruebas
# Útil para ver la salida en tiempo real

set -e

echo "=== Ejecutando Sensor Daemon en Foreground ==="
echo "Presiona Ctrl+C para detener"
echo

# Verificar que el binario existe
if [ ! -f "../build/sensor_daemon" ]; then
    echo "ERROR: El binario no existe. Ejecuta 'make' primero."
    exit 1
fi

# Limpiar logs anteriores
rm -f /tmp/sensor.log /var/tmp/sensor.log

echo "Iniciando daemon con intervalo de 2 segundos..."
echo "Los logs se escribirán en /tmp/sensor.log o /var/tmp/sensor.log"
echo

# Ejecutar el daemon con intervalo de 2 segundos
../build/sensor_daemon 2
