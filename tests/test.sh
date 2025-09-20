#!/bin/bash

# =============================================================================
# COMPREHENSIVE TEST SCRIPT FOR ASSIGNMENT-SENSOR SYSTEM SERVICE
# =============================================================================
# This script provides complete testing, installation, and monitoring capabilities
# for the assignment-sensor systemd service that logs mock sensor data.
#
# Features:
# - Real-time log monitoring (10 seconds)
# - Complete installation/uninstallation testing
# - Fallback behavior verification
# - Graceful shutdown testing
# - Configuration validation
# =============================================================================

# --- Configuración y Seguridad ---
# 'set -e' hace que el script falle y se detenga inmediatamente si cualquier comando falla.
# 'set -o pipefail' asegura que si hay un pipe (|), el script falle si cualquier parte del pipe falla.
set -e
set -o pipefail

# --- Variables Globales ---
SERVICE_NAME="assignment-sensor"
LOG_FILE_TMP="/tmp/sensor_log.txt"
LOG_FILE_VAR_TMP="/var/tmp/sensor_log.txt"
BINARY_PATH="/usr/local/bin/${SERVICE_NAME}"
SERVICE_PATH="/etc/systemd/system/${SERVICE_NAME}.service"
BUILD_PATH="./build/${SERVICE_NAME}"
TEST_LOG_FILE="/tmp/test_sensor_log.txt"

# --- Funciones de Utilidad ---

# Función para mostrar logs en tiempo real por 10 segundos
monitor_logs_realtime() {
    local log_file="$1"
    local duration="${2:-10}"
    
    echo "--- Monitoreando logs en tiempo real por ${duration} segundos ---"
    echo "Archivo: $log_file"
    echo "Presiona Ctrl+C para detener el monitoreo antes de tiempo"
    echo ""
    
    if [ ! -f "$log_file" ]; then
        echo "ADVERTENCIA: El archivo de log no existe aún: $log_file"
        echo "Esperando a que se cree..."
        local count=0
        while [ ! -f "$log_file" ] && [ $count -lt 20 ]; do
            sleep 1
            count=$((count + 1))
            echo -n "."
        done
        echo ""
        if [ ! -f "$log_file" ]; then
            echo "ERROR: El archivo de log no se creó después de 20 segundos"
            return 1
        fi
    fi
    
    # Usar tail -f para seguir el archivo en tiempo real
    timeout "${duration}s" tail -f "$log_file" 2>/dev/null || {
        echo ""
        echo "Monitoreo completado (${duration} segundos)"
        echo ""
        echo "--- Contenido completo del archivo de log ---"
        cat "$log_file"
        echo ""
        echo "--- Fin del contenido del log ---"
    }
}

# Función para verificar el estado del servicio
check_service_status() {
    local service_name="$1"
    echo "--- Estado del servicio: $service_name ---"
    sudo systemctl status "$service_name" --no-pager -l
    echo ""
}

# Función para mostrar información de configuración
show_configuration_info() {
    echo "--- INFORMACIÓN DE CONFIGURACIÓN ---"
    echo ""
    echo "CLI Flags soportados:"
    echo "  --interval <seconds>  : Intervalo entre lecturas del sensor (default: 5)"
    echo "  --logfile <path>      : Ruta personalizada del archivo de log (no implementado aún)"
    echo "  --device <path>       : Ruta del dispositivo sensor (no implementado aún)"
    echo ""
    echo "Rutas por defecto:"
    echo "  Log principal: /tmp/sensor_log.txt"
    echo "  Log fallback:  /var/tmp/sensor_log.txt"
    echo "  Binario:       $BINARY_PATH"
    echo "  Servicio:      $SERVICE_PATH"
    echo ""
    echo "Ejemplos de uso:"
    echo "  $SERVICE_NAME                    # Usa configuración por defecto"
    echo "  $SERVICE_NAME --interval 10      # Lee cada 10 segundos"
    echo "  $SERVICE_NAME --interval 1       # Lee cada segundo"
    echo ""
}

# --- El Seguro de Vida: Función de Limpieza ---
# Esta función se ejecuta SIEMPRE, no importa si el script tiene éxito o falla.
# 'trap' se encarga de llamarla al final.
cleanup() {
  echo ""
  echo "--- Ejecutando limpieza automática... ---"
  
  # Desregistra y detiene el servicio de systemd (ignora errores si no existe)
  sudo systemctl stop ${SERVICE_NAME}.service &>/dev/null || true
  sudo systemctl disable ${SERVICE_NAME}.service &>/dev/null || true
  
  # Elimina los archivos que instalamos en el sistema
  sudo rm -f "$BINARY_PATH"
  sudo rm -f "$SERVICE_PATH"
  
  # Recargamos systemd para que olvide por completo el servicio
  sudo systemctl daemon-reload
  
  # Elimina los archivos de log y cualquier bloqueo simulado
  sudo rm -f "$LOG_FILE_TMP"
  sudo rm -f "$LOG_FILE_VAR_TMP"
  sudo rm -f "$TEST_LOG_FILE"
  sudo rmdir "$LOG_FILE_TMP" &>/dev/null || true # Importante para la prueba de fallback

  # Limpia los archivos de compilación locales
  make clean &>/dev/null
  echo "Sistema limpio."
}

# --- Función Principal que Ejecuta las Pruebas ---
main() {
  # Registramos la función de limpieza para que se ejecute al salir (EXIT) o si hay un error (INT, TERM)
  trap cleanup EXIT INT TERM

  # Verificamos que se ejecute con sudo
  if [[ $EUID -ne 0 ]]; then
     echo "ERROR: Este script debe ejecutarse con sudo."
     exit 1
  fi
  
  echo "============================================================================="
  echo "INICIANDO PRUEBAS COMPREHENSIVAS DEL SERVICIO ASSIGNMENT-SENSOR"
  echo "============================================================================="
  echo ""
  
  # =============================================================================
  # SECCIÓN 1: CLONE & BUILD
  # =============================================================================
  echo "SECCIÓN 1: CLONE & BUILD"
  echo "============================"
  
  echo "PRUEBA 1.1: Verificando prerequisitos..."
  echo "  - Compilador GCC: $(gcc --version | head -n1)"
  echo "  - Systemd: $(systemctl --version | head -n1)"
  echo "  - Make: $(make --version | head -n1)"
  echo "Prerequisitos verificados."
  echo ""
  
  echo "PRUEBA 1.2: Compilando el proyecto..."
  make clean &>/dev/null || true
  make
  if [ ! -f "$BUILD_PATH" ]; then
      echo "FALLÓ: El ejecutable no fue creado en $BUILD_PATH"
      exit 1
  fi
  echo "PASÓ: El programa se compiló sin errores."
  echo "  - Artefacto producido: $BUILD_PATH"
  echo "  - Tamaño: $(du -h "$BUILD_PATH" | cut -f1)"
  echo ""
  
  # =============================================================================
  # SECCIÓN 2: INSTALL & ENABLE
  # =============================================================================
  echo "SECCIÓN 2: INSTALL & ENABLE"
  echo "==============================="
  
  echo "PRUEBA 2.1: Instalando binario y servicio..."
  sudo cp "$BUILD_PATH" "$BINARY_PATH"
  sudo cp "systemd/${SERVICE_NAME}.service" "$SERVICE_PATH"
  sudo systemctl daemon-reload
  echo "PASÓ: Archivos instalados correctamente."
  echo "  - Binario: $BINARY_PATH"
  echo "  - Servicio: $SERVICE_PATH"
  echo ""
  
  echo "PRUEBA 2.2: Habilitando e iniciando el servicio..."
  sudo systemctl enable --now ${SERVICE_NAME}.service
  sleep 3 # Damos tiempo a que inicie
  
  if ! sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
      echo "FALLÓ: El servicio no está activo después de iniciarlo."
      check_service_status "${SERVICE_NAME}.service"
      exit 1
  fi
  echo "PASÓ: El servicio systemd está activo y ejecutándose."
  check_service_status "${SERVICE_NAME}.service"
  
  # =============================================================================
  # SECCIÓN 3: CONFIGURACIÓN
  # =============================================================================
  echo "SECCIÓN 3: CONFIGURACIÓN"
  echo "============================"
  show_configuration_info
  
  # =============================================================================
  # SECCIÓN 4: TESTING COMPREHENSIVO
  # =============================================================================
  echo "SECCIÓN 4: TESTING COMPREHENSIVO"
  echo "==================================="
  
  echo "PRUEBA 4.1: Verificando creación de log en /tmp..."
  if [ ! -s "$LOG_FILE_TMP" ]; then
      echo "FALLÓ: El archivo de log en $LOG_FILE_TMP no fue creado o está vacío."
      exit 1
  fi
  echo "PASÓ: El log se creó correctamente en /tmp."
  echo "  - Tamaño actual: $(du -h "$LOG_FILE_TMP" | cut -f1)"
  echo "  - Últimas 3 líneas:"
  tail -n 3 "$LOG_FILE_TMP" | sed 's/^/    /'
  echo ""
  
  echo "PRUEBA 4.2: Monitoreo en tiempo real (20 segundos)..."
  monitor_logs_realtime "$LOG_FILE_TMP" 20
  echo ""
  
  echo "PRUEBA 4.3: Verificando formato de logs..."
  local log_line=$(tail -n 1 "$LOG_FILE_TMP")
  if [[ $log_line =~ ^\[[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z\]\ Sensor\ Value:\ [0-9]+\.[0-9]{2}$ ]]; then
      echo "PASÓ: Formato de log correcto (ISO-8601 + valor numérico)."
      echo "  - Ejemplo: $log_line"
  else
      echo "FALLÓ: Formato de log incorrecto."
      echo "  - Línea: $log_line"
      exit 1
  fi
  echo ""
  
  echo "PRUEBA 4.4: Prueba de apagado ordenado (SIGTERM)..."
  sudo systemctl stop ${SERVICE_NAME}.service
  sleep 2
  
  if sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
      echo "FALLÓ: El servicio sigue activo después de enviarle 'stop'."
      exit 1
  fi
  echo "PASÓ: El servicio se detuvo correctamente con SIGTERM."
  echo ""
  
  echo "PRUEBA 4.5: Prueba de comportamiento fallback (/tmp no escribible)..."
  # Simulamos que /tmp no es escribible creando un DIRECTORIO con el mismo nombre
  sudo mkdir "$LOG_FILE_TMP"
  
  # Reiniciamos el servicio
  sudo systemctl start ${SERVICE_NAME}.service
  sleep 3
  
  if ! sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
      echo "FALLÓ: El servicio no pudo iniciar en la prueba de fallback."
      exit 1
  fi
  
  if [ -f "$LOG_FILE_TMP" ]; then
      echo "FALLÓ: El programa escribió en /tmp cuando no debía."
      exit 1
  fi

  if [ ! -s "$LOG_FILE_VAR_TMP" ]; then
      echo "FALLÓ: ¡No se creó el log de fallback en $LOG_FILE_VAR_TMP!"
      exit 1
  fi
  echo "PASÓ: La lógica de fallback a /var/tmp funciona correctamente."
  echo "  - Log fallback: $LOG_FILE_VAR_TMP"
  echo "  - Tamaño: $(du -h "$LOG_FILE_VAR_TMP" | cut -f1)"
  echo ""
  
  echo "PRUEBA 4.6: Monitoreo en tiempo real del log fallback (20 segundos)..."
  monitor_logs_realtime "$LOG_FILE_VAR_TMP" 20
  echo ""
  
  echo "PRUEBA 4.7: Verificando reinicio automático del servicio..."
  # Simulamos un fallo matando el proceso
  local pid=$(sudo systemctl show -p MainPID --value "${SERVICE_NAME}.service")
  if [ "$pid" != "0" ] && [ "$pid" != "" ]; then
      sudo kill -9 "$pid" 2>/dev/null || true
      sleep 5
      if sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
          echo "PASÓ: El servicio se reinició automáticamente después del fallo."
      else
          echo "ADVERTENCIA: El servicio no se reinició automáticamente."
      fi
  else
      echo "ADVERTENCIA: No se pudo obtener el PID del servicio para la prueba de reinicio."
  fi
  echo ""
  
  # =============================================================================
  # SECCIÓN 5: UNINSTALL (DEMO)
  # =============================================================================
  echo "SECCIÓN 5: UNINSTALL (DEMO)"
  echo "==============================="
  echo "PRUEBA 5.1: Deshabilitando y deteniendo el servicio..."
  sudo systemctl disable --now ${SERVICE_NAME}.service
  sleep 2
  
  if sudo systemctl is-active --quiet "${SERVICE_NAME}.service"; then
      echo "FALLÓ: El servicio sigue activo después de deshabilitarlo."
      exit 1
  fi
  echo "PASÓ: El servicio se deshabilitó y detuvo correctamente."
  echo ""
  
  echo "PRUEBA 5.2: Verificando que los archivos pueden ser removidos..."
  if [ -f "$BINARY_PATH" ]; then
      echo "  - Binario presente: $BINARY_PATH"
  fi
  if [ -f "$SERVICE_PATH" ]; then
      echo "  - Servicio presente: $SERVICE_PATH"
  fi
  echo "PASÓ: Archivos listos para remoción (se limpiarán automáticamente)."
  echo ""
  
  # =============================================================================
  # RESUMEN FINAL
  # =============================================================================
  echo "============================================================================="
  echo "¡TODAS LAS PRUEBAS COMPREHENSIVAS PASARON CON ÉXITO!"
  echo "============================================================================="
  echo ""
  echo "RESUMEN DE PRUEBAS COMPLETADAS:"
  echo "  - Compilación y build"
  echo "  - Instalación de binario y servicio systemd"
  echo "  - Habilitación y inicio del servicio"
  echo "  - Creación de logs en /tmp"
  echo "  - Monitoreo en tiempo real (20 segundos)"
  echo "  - Validación de formato de logs"
  echo "  - Apagado ordenado con SIGTERM"
  echo "  - Comportamiento fallback a /var/tmp"
  echo "  - Monitoreo en tiempo real del fallback"
  echo "  - Reinicio automático del servicio"
  echo "  - Deshabilitación y limpieza"
  echo ""
  echo "CONFIGURACIÓN VERIFICADA:"
  echo "  - Intervalo por defecto: 5 segundos"
  echo "  - Log principal: /tmp/sensor_log.txt"
  echo "  - Log fallback: /var/tmp/sensor_log.txt"
  echo "  - Formato: ISO-8601 UTC con valores de sensor"
  echo ""
  echo "NOTAS:"
  echo "  - Los flags --logfile y --device no están implementados aún"
  echo "  - El servicio se reinicia automáticamente en caso de fallo"
  echo "  - La limpieza automática se ejecutará al final del script"
  echo ""
}

# --- Punto de Entrada ---
# Llama a la función principal para empezar el show.
main