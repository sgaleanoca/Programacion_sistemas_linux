# Programación de Sistemas Linux Embebidos

Repositorio maestro que contiene todos los proyectos de la materia de Programación de Sistemas Linux Embebidos.

## 📋 Proyectos

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

### 2. Sistema de Servicio con Sensor Mock
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

## 🚀 Cómo usar este repositorio

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
   ```

3. **Seguir las instrucciones del README específico** de cada proyecto

## 📁 Estructura del repositorio

```
Programacion_sistemas_linux/
├── README.md                           # Este archivo
├── proyecto-1-sistema-monitoreo/       # Sistema de monitoreo
│   ├── src/                           # Código fuente
│   ├── include/                       # Archivos de cabecera
│   ├── Makefile                       # Compilación
│   └── README.md                      # Documentación del proyecto
├── proyecto-2-embedded-session-hw/    # Proyecto de hardware embebido
└── proyecto-3-system-service-that-logs-a-mock-sensor/  # Servicio de sensor
    ├── src/                           # Código fuente
    ├── systemd/                       # Configuración del servicio
    ├── tests/                         # Pruebas automatizadas
    ├── build/                         # Artefactos compilados
    ├── Makefile                       # Configuración de compilación
    └── README.md                      # Documentación del proyecto
```

## 🛠️ Herramientas utilizadas

- **Lenguaje**: C
- **Compilador**: GCC
- **Sistema**: Linux
- **Control de versiones**: Git
- **Plataforma**: GitHub
- **Servicios**: systemd

## 📚 Conceptos aprendidos

- Programación en C para sistemas Linux
- Lectura de archivos del sistema (`/proc/`)
- Manejo de memoria y CPU
- Compilación con Makefile
- Control de versiones con Git
- Documentación técnica
- Servicios systemd
- Manejo de señales
- Logging y monitoreo

## 👨‍💻 Autor

**Santiago Galeano Caicedo**
- GitHub: [@sgaleanoca](https://github.com/sgaleanoca)

---

*Este repositorio contiene proyectos académicos de la materia de Programación de Sistemas Linux Embebidos.*
