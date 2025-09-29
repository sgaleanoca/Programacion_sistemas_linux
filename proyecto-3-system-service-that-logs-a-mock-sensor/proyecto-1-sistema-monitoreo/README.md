# Sistema de Monitoreo del Sistema

Un programa en C que monitorea en tiempo real la información del CPU y memoria de tu computadora Linux.

## ¿Qué hace este programa?

Este programa te muestra información actualizada cada 2 segundos sobre:

- **Memoria RAM**: Total, libre, disponible y usada
- **Memoria Swap**: Total, libre y usada (memoria virtual)
- **⚡ CPU**: Modelo del procesador, número de cores y carga de cada core


## Estructura del proyecto

```
proyecto-sistema/
├── include/           # Archivos de cabecera (.h)
│   ├── cpu.h         # Definiciones para funciones del CPU
│   └── memory.h      # Definiciones para funciones de memoria
├── src/              # Código fuente (.c)
│   ├── main.c        # Programa principal
│   ├── cpu.c         # Funciones para obtener info del CPU
│   └── memory.c      # Funciones para obtener info de memoria
├── Makefile          # Archivo para compilar automáticamente
└── README.md         # Esta documentación
```

## Cómo funciona el programa

### Flujo principal (`main.c`):
1. **Inicialización**: Obtiene información básica del CPU
2. **Loop infinito**: Se ejecuta continuamente
3. **Limpieza**: Borra la pantalla cada 2 segundos
4. **Actualización**: Obtiene y muestra información actualizada
5. **Pausa**: Espera 2 segundos antes de repetir

### Funciones del CPU (`cpu.c`):
- **`get_cpu_info()`**: Lee `/proc/cpuinfo` para obtener modelo y número de cores
- **`print_cpu_info()`**: Muestra la información básica del CPU
- **`get_cpu_load_per_core()`**: Lee `/proc/stat` para calcular la carga de cada core

### Funciones de memoria (`memory.c`):
- **`get_memory_info()`**: Lee `/proc/meminfo` para obtener información de RAM y swap
- **`print_memory_info()`**: Muestra toda la información de memoria

## Ejemplo de salida

```
Memoria total: 15696620 KB
Memoria libre: 1752960 KB
Memoria disponible: 8699824 KB
Memoria swap libre: 4194300 KB
Memoria swap usada: 0 KB
Procesador: AMD Ryzen 5 5600H with Radeon Graphics
Cores: 12
Core 0: 2.74%
Core 1: 1.29%
Core 2: 3.90%
Core 3: 1.68%
Core 4: 4.92%
Core 5: 1.98%
Core 6: 8.28%
Core 7: 5.21%
Core 8: 8.30%
Core 9: 4.98%
Core 10: 3.89%
Core 11: 1.23%

```

## Archivos del sistema que lee el programa

- **`/proc/cpuinfo`**: Información del procesador
- **`/proc/stat`**: Estadísticas del CPU en tiempo real
- **`/proc/meminfo`**: Información de memoria RAM y swap

