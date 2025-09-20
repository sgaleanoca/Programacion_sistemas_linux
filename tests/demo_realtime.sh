#!/bin/bash

# =============================================================================
# DEMO SCRIPT: Real-time Log Monitoring for Assignment-Sensor
# =============================================================================
# This script demonstrates the real-time monitoring functionality
# without running the full test suite.
# =============================================================================

set -e

SERVICE_NAME="assignment-sensor"
LOG_FILE_TMP="/tmp/sensor_log.txt"
LOG_FILE_VAR_TMP="/var/tmp/sensor_log.txt"

# Function to monitor logs in real-time
monitor_logs_realtime() {
    local log_file="$1"
    local duration="${2:-10}"
    
    echo "--- 📊 Monitoreando logs en tiempo real por ${duration} segundos ---"
    echo "Archivo: $log_file"
    echo "Presiona Ctrl+C para detener el monitoreo antes de tiempo"
    echo ""
    
    if [ ! -f "$log_file" ]; then
        echo "⚠️  El archivo de log no existe aún: $log_file"
        echo "Esperando a que se cree..."
        local count=0
        while [ ! -f "$log_file" ] && [ $count -lt 20 ]; do
            sleep 1
            count=$((count + 1))
            echo -n "."
        done
        echo ""
        if [ ! -f "$log_file" ]; then
            echo "❌ El archivo de log no se creó después de 20 segundos"
            return 1
        fi
    fi
    
    # Show current content first
    echo "--- Contenido actual del log ---"
    if [ -s "$log_file" ]; then
        tail -n 5 "$log_file"
    else
        echo "(archivo vacío)"
    fi
    echo ""
    
    # Use tail -f to follow the file in real-time
    echo "--- Iniciando monitoreo en tiempo real ---"
    timeout "${duration}s" tail -f "$log_file" 2>/dev/null || {
        echo ""
        echo "✅ Monitoreo completado (${duration} segundos)"
    }
}

# Main demo function
demo() {
    echo "============================================================================="
    echo "🎬 DEMO: Monitoreo en Tiempo Real de Assignment-Sensor"
    echo "============================================================================="
    echo ""
    
    # Check if service is running
    if ! sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
        echo "⚠️  El servicio ${SERVICE_NAME} no está ejecutándose."
        echo "Para ejecutar este demo, primero inicia el servicio:"
        echo "  sudo systemctl start ${SERVICE_NAME}.service"
        echo ""
        echo "O ejecuta el script completo de pruebas:"
        echo "  sudo ./tests/test.sh"
        exit 1
    fi
    
    echo "✅ Servicio ${SERVICE_NAME} está ejecutándose."
    echo ""
    
    # Check which log file is being used
    if [ -f "$LOG_FILE_TMP" ]; then
        echo "📁 Usando log principal: $LOG_FILE_TMP"
        monitor_logs_realtime "$LOG_FILE_TMP" 10
    elif [ -f "$LOG_FILE_VAR_TMP" ]; then
        echo "📁 Usando log fallback: $LOG_FILE_VAR_TMP"
        monitor_logs_realtime "$LOG_FILE_VAR_TMP" 10
    else
        echo "❌ No se encontraron archivos de log."
        echo "Verifica que el servicio esté funcionando correctamente."
        exit 1
    fi
    
    echo ""
    echo "🎉 Demo completado!"
    echo ""
    echo "💡 Para más pruebas, ejecuta: sudo ./tests/test.sh"
}

# Run demo
demo
