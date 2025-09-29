#include <stdio.h>
#include <string.h>
#include "cpu.h"

// Función que obtiene la info del CPU
CPUInfo get_cpu_info() {                                                                // Obtiene la info del CPU
    CPUInfo cpu;                                                                        // Estructura para guardar info del CPU
    FILE *fp = fopen("/proc/cpuinfo", "r");                                             // Abre el archivo /proc/cpuinfo
    char line[256];                                                                     // Linea para leer el archivo 
    cpu.cores = 0;                                                                      // Inicializa el numero de cores

    if (!fp) {                                                                          // Si no se pudo abrir el archivo
        perror("No se pudo abrir /proc/cpuinfo");                                       // Manejo de errores
        return cpu;                                                                     // Retorna la estructura del CPU
    }

    while (fgets(line, sizeof(line), fp)) {                                             // Lee el archivo linea por linea
        if (strncmp(line, "model name", 10) == 0) {                                     // Si la linea es "model name"
            // Solo la primera vez guardamos el nombre
            if (cpu.cores == 0) {                                                       // Solo la primera vez guardamos el nombre
                sscanf(line, "model name : %[^\n]", cpu.model_name);                    // Guarda el nombre del CPU
            }
            cpu.cores++;                                                                // Incrementa el numero de cores
        }
    }

    fclose(fp);                                                                         // Cierra el archivo
    return cpu;                                                                         // Retorna la estructura del CPU
}

// Imprime la info del CPU
void print_cpu_info(CPUInfo cpu) {                                                      // Imprime la info del CPU
    printf("Procesador: %s\n", cpu.model_name);                                         // Imprime el nombre del CPU
    printf("Cores: %d\n", cpu.cores);                                                   // Imprime el numero de cores
}

// Función simple para calcular carga por core
void get_cpu_load_per_core(float *loads, int cores) {                                   // Obtiene la carga del CPU por core  
    FILE *fp = fopen("/proc/stat", "r");                                                // Abre el archivo /proc/stat
    char line[256];                                                                     // Linea para leer el archivo
    int core_index = 0;                                                                 // Indice para el core

    if (!fp) {                                                                          // Si no se pudo abrir el archivo
        perror("No se pudo abrir /proc/stat");                                          // Manejo de errores
        return;                                                                         // Retorna  
    }

    while (fgets(line, sizeof(line), fp) && core_index < cores) {                       // Lee el archivo linea por linea
        if (strncmp(line, "cpu", 3) == 0 && line[3] != ' ') {                           // Si la linea es "cpu"
            long user, nice, system, idle;                                              // Variables para guardar los datos
            sscanf(line, "cpu%*d %ld %ld %ld %ld", &user, &nice, &system, &idle);       // Guarda los datos
            long total = user + nice + system + idle;                                   // Calcula el total
            loads[core_index] = (float)(total - idle) / total * 100.0;                  // % de uso
            core_index++;                                                               // Incrementa el indice del core
        }
    }

    fclose(fp);                                                                         // Cierra el archivo
}
