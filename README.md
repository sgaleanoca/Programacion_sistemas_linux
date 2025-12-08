# Programación de Sistemas Linux Embebidos

Repositorio maestro que contiene todos los proyectos de la materia de Programación de Sistemas Linux Embebidos.

## Proyectos

### 1. Sistema de Monitoreo del Sistema
- **Descripción**: Programa en C que monitorea en tiempo real la información del CPU y memoria de sistemas Linux
- **Lenguaje**: C
- **Carpeta**: [proyecto-1-sistema-monitoreo](./proyecto-1-sistema-monitoreo/)
- **Funcionalidades**:
  - Monitoreo de memoria RAM (total, libre, disponible)
  - Monitoreo de memoria swap
  - Información del procesador y número de cores
  - Carga de CPU por cada core individual
  - Actualización en tiempo real cada 2 segundos

### 2. Sistema Embebido con Hardware
- **Descripción**: Proyecto de hardware embebido con sensores y actuadores
- **Lenguaje**: C
- **Carpeta**: [proyecto-2-embedded-session-hw](./proyecto-2-embedded-session-hw/)
- **Componentes**:
  - Sensores para lectura de datos del entorno
  - Actuadores (LEDs, buzzer) para respuesta física
  - Controlador principal que gestiona la interacción entre sensores y actuadores

### 3. Sistema de Servicio con Sensor Mock
- **Descripción**: Servicio systemd que registra datos de sensores simulados con comportamiento de respaldo y manejo de apagado elegante
- **Lenguaje**: C
- **Carpeta**: [proyecto-3-system-service-that-logs-a-mock-sensor](./proyecto-3-system-service-that-logs-a-mock-sensor/)
- **Funcionalidades**:
  - Registro en tiempo real de datos de sensores cada 5 segundos (configurable)
  - Simulación de sensor mock que genera valores aleatorios de temperatura entre 20.0°C y 30.0°C
  - Comportamiento de respaldo: cambia automáticamente a `/var/tmp` si `/tmp` no es escribible
  - Apagado elegante: maneja la señal SIGTERM correctamente
  - Timestamps ISO-8601: formato UTC para todas las entradas de log
  - Integración con systemd: servicio completo con capacidades de reinicio automático
  - Pruebas exhaustivas: suite de pruebas automatizada que cubre toda la funcionalidad

### 4. Controlador Gamepad Bluetooth (ESP32)
- **Descripción**: Controlador de juego DIY basado en ESP32 que funciona como dispositivo HID a través de Bluetooth Low Energy (BLE)
- **Lenguaje**: C (ESP-IDF)
- **Carpeta**: [proyecto-4-gamepad](./proyecto-4-gamepad/)
- **Hardware**:
  - ESP32 como microcontrolador principal
  - 4 botones digitales (A, B, SELECT, START)
  - Joystick analógico de 2 ejes
- **Funcionalidades**:
  - Implementación del perfil HID estándar sobre BLE
  - Lectura continua de botones y joystick
  - Transmisión inalámbrica de datos de gamepad
  - Compatible con sistemas operativos que soporten BLE HID

### 5. Mouse Bluetooth (ESP32)
- **Descripción**: Dispositivo mouse inalámbrico implementado con ESP32 usando BLE HID
- **Lenguaje**: C (ESP-IDF)
- **Carpeta**: [proyecto-6-bluetooth-mouse](./proyecto-6-bluetooth-mouse/)
- **Funcionalidades**:
  - Implementación del perfil HID Mouse sobre BLE
  - Control de movimiento del cursor
  - Compatible con dispositivos que soporten BLE HID
  - Basado en el ejemplo BLE HID de ESP-IDF

### 6. Proyecto Final - Consola de Juegos con Raspberry Pi y Controlador Bluetooth DIY

- **Descripción**: Sistema completo de consola de juegos embebida que integra una Raspberry Pi Zero 2W ejecutando PICO-8 con un controlador de juego DIY basado en ESP32
- **Carpeta**: [proyecto-final](./proyecto-final/)
- **Componentes principales**:

#### 6.1 Controlador Bluetooth DIY (ESP32)
- **Ubicación**: `proyecto-final/proyecto-5-bluetooth-controller-2.0/`
- **Descripción**: Controlador de juego completo con 4 botones (A, B, SELECT, START) y joystick analógico de 2 ejes
- **Tecnología**: 
  - ESP32 con ESP-IDF
  - Bluetooth Low Energy (BLE) HID Profile
  - FreeRTOS para multitarea
- **Características técnicas**:
  - Calibración automática del joystick
  - Zona muerta configurable para evitar drift
  - Curva de respuesta suave para mejor control
  - Detección de cambios para optimizar el envío de datos
  - Frecuencia de muestreo: 20 Hz (50ms por muestra)
- **Hardware**:
  - Botones: GPIO 13 (A), GPIO 12 (B), GPIO 25 (SELECT), GPIO 26 (START)
  - Joystick: GPIO 36 (VRX - ADC1_CH0), GPIO 39 (VRY - ADC1_CH3)
  - ADC de 12 bits con atenuación de 11 dB

#### 6.2 Juego MagicPicoCat
- **Ubicación**: `proyecto-final/mpc/`
- **Descripción**: Clon del juego "Magic Cat Academy" desarrollado para PICO-8
- **Archivos**:
  - `magic_pico_cat.p8`: Juego principal
  - `mca.p8`: Versión alternativa
- **Plataforma**: PICO-8 (fantasy console)

#### 6.3 Configuración de Raspberry Pi Zero 2W
- **Ubicación**: `proyecto-final/Paso a paso para instalar pico-8 en la RPI.md`
- **Descripción**: Guía completa para configurar la Raspberry Pi Zero 2W como consola de juegos
- **Pasos incluidos**:
  - Instalación de dependencias del sistema
  - Configuración de auto-login
  - Instalación de PICO-8
  - Configuración de inicio automático en modo kiosco
  - Solución de problemas de compatibilidad ARM (32-bit vs 64-bit)
  - Instalación de librerías SDL2 necesarias

#### 6.4 Funcionamiento del Sistema Completo

El proyecto final integra todos los componentes en un sistema funcional:

1. **Raspberry Pi Zero 2W**: Ejecuta PICO-8 en modo kiosco, iniciando automáticamente al encender
2. **Controlador ESP32**: Se conecta vía Bluetooth BLE al sistema operativo Linux de la Raspberry Pi
3. **Reconocimiento HID**: Linux reconoce automáticamente el controlador como gamepad estándar
4. **Integración con PICO-8**: PICO-8 detecta y mapea automáticamente los controles del gamepad
5. **Experiencia de juego**: Permite jugar MagicPicoCat usando el controlador DIY de forma inalámbrica

#### 6.5 Documentación Adicional
- `Proyecto Final - Linux Embebidos.pdf`: Documentación técnica del proyecto
- `Proyecto_FInal_Linux.pdf`: Documentación adicional

## Cómo usar este repositorio

1. **Clonar el repositorio**:
   ```bash
   git clone https://github.com/sgaleanoca/Programacion_sistemas_linux.git
   cd Programacion_sistemas_linux
   ```

2. **Navegar al proyecto deseado**:
   ```bash
   cd proyecto-1-sistema-monitoreo
   # o
   cd proyecto-3-system-service-that-logs-a-mock-sensor
   # o
   cd proyecto-final
   ```

3. **Seguir las instrucciones del README específico** de cada proyecto

## Estructura del repositorio

```
Programacion_sistemas_linux/
├── README.md                                    # Este archivo
├── proyecto-1-sistema-monitoreo/                # Sistema de monitoreo de CPU y memoria
│   ├── src/                                    # Código fuente
│   ├── include/                                # Archivos de cabecera
│   ├── Makefile                                # Compilación
│   └── README.md                               # Documentación del proyecto
├── proyecto-2-embedded-session-hw/             # Proyecto de hardware embebido
│   └── embedded-session-hw/
│       ├── actuators/                          # Actuadores (LEDs, buzzer)
│       ├── sensor/                             # Sensores
│       ├── controller/                         # Controlador principal
│       └── tests/                              # Pruebas
├── proyecto-3-system-service-that-logs-a-mock-sensor/  # Servicio systemd de sensor
│   ├── src/                                    # Código fuente
│   ├── systemd/                                # Configuración del servicio
│   ├── tests/                                  # Pruebas automatizadas
│   ├── build/                                  # Artefactos compilados
│   ├── Makefile                                # Configuración de compilación
│   └── README.md                               # Documentación del proyecto
├── proyecto-4-gamepad/                         # Controlador gamepad Bluetooth (ESP32)
│   ├── main/                                   # Código fuente ESP-IDF
│   ├── build/                                  # Artefactos compilados
│   ├── CMakeLists.txt                          # Configuración CMake
│   └── README.md                               # Documentación del proyecto
├── proyecto-6-bluetooth-mouse/                  # Mouse Bluetooth (ESP32)
│   ├── main/                                   # Código fuente ESP-IDF
│   ├── build/                                  # Artefactos compilados
│   ├── CMakeLists.txt                          # Configuración CMake
│   └── README.md                               # Documentación del proyecto
└── proyecto-final/                             # PROYECTO FINAL - Consola de juegos completa
    ├── proyecto-5-bluetooth-controller-2.0/    # Controlador Bluetooth DIY (ESP32)
    │   ├── main/                               # Código fuente del controlador
    │   │   ├── controller.c/h                  # Gestión de botones y joystick
    │   │   ├── ble_hidd_demo_main.c            # Función principal y eventos BLE
    │   │   └── esp_hidd_prf_api.c/h            # API del perfil HID BLE
    │   ├── CMakeLists.txt                      # Configuración ESP-IDF
    │   └── README.md                           # Documentación completa del controlador
    ├── mpc/                                    # Juego MagicPicoCat para PICO-8
    │   ├── magic_pico_cat.p8                   # Juego principal
    │   ├── mca.p8                              # Versión alternativa
    │   └── README.md                           # Documentación del juego
    ├── Paso a paso para instalar pico-8 en la RPI.md  # Guía de instalación
    ├── Proyecto Final - Linux Embebidos.pdf   # Documentación técnica
    └── Proyecto_FInal_Linux.pdf                # Documentación adicional
```

## Herramientas utilizadas

- **Lenguajes**: C, Lua (PICO-8)
- **Compiladores**: GCC, ESP-IDF toolchain
- **Sistemas**: Linux (Raspberry Pi OS), FreeRTOS (ESP32)
- **Frameworks**: ESP-IDF (Espressif IoT Development Framework)
- **Control de versiones**: Git
- **Plataforma**: GitHub
- **Servicios**: systemd
- **Hardware**: Raspberry Pi Zero 2W, ESP32
- **Consola de juegos**: PICO-8

## Conceptos aprendidos

### Programación en Linux
- Programación en C para sistemas Linux
- Lectura de archivos del sistema (`/proc/`)
- Manejo de memoria y CPU
- Compilación con Makefile
- Servicios systemd
- Manejo de señales (SIGTERM)
- Logging y monitoreo
- Configuración de sistemas embebidos

### Programación Embebida
- Programación de microcontroladores ESP32
- Uso de ESP-IDF framework
- FreeRTOS para multitarea
- GPIO y ADC (conversión analógica-digital)
- Bluetooth Low Energy (BLE)
- Perfil HID sobre BLE
- Calibración de sensores analógicos
- Gestión de energía en sistemas embebidos

### Desarrollo de Sistemas Complejos
- Integración hardware-software
- Comunicación inalámbrica (BLE)
- Protocolos estándar (HID)
- Configuración de sistemas Linux embebidos
- Modo kiosco y auto-inicio
- Desarrollo de juegos en PICO-8
- Documentación técnica completa

