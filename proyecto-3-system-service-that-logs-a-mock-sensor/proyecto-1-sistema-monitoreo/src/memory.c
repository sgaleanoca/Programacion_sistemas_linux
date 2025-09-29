#include <stdio.h>
#include "memory.h"

MemoryInfo get_memory_info() {                                                              // Obtiene la info de la memoria
    MemoryInfo mem = {0};                                                                   // Estructura para guardar info de la memoria
    FILE *fp = fopen("/proc/meminfo", "r");                                                 // Abre el archivo /proc/meminfo
    char line[256];                                                                         // Linea para leer el archivo

    if (!fp) {                                                                              // Si no se pudo abrir el archivo
        perror("No se pudo abrir /proc/meminfo");                                           // Manejo de errores
        return mem;                                                                         // Retorna la estructura de la memoria
    }

    while (fgets(line, sizeof(line), fp)) {                                                 // Lee el archivo linea por linea
        if (sscanf(line, "MemTotal: %ld kB", &mem.total) == 1) continue;                    // Guarda el total de memoria
        if (sscanf(line, "MemFree: %ld kB", &mem.free) == 1) continue;                      // Guarda la memoria libre
        if (sscanf(line, "MemAvailable: %ld kB", &mem.available) == 1) continue;            // Guarda la memoria disponible
        if (sscanf(line, "SwapFree: %ld kB", &mem.swap_free) == 1) continue;                // Guarda la memoria swap libre
        if (sscanf(line, "SwapTotal: %ld kB", &mem.swap_total) == 1) continue;              // Guarda la memoria swap total
    }

    fclose(fp);                                                                             // Cierra el archivo
    mem.used = mem.total - mem.free;                                                        // física usada
    mem.swap_used = mem.swap_total - mem.swap_free;                                         // virtual usada
    return mem;                                                                             // Retorna la estructura de la memoria

}

void print_memory_info(MemoryInfo mem) {                                                    // Imprime la info de la memoria
    printf("Memoria total: %ld KB\n", mem.total);                                           // Imprime la memoria total   
    printf("Memoria libre: %ld KB\n", mem.free);                                            // Imprime la memoria libre
    printf("Memoria disponible: %ld KB\n", mem.available);                                  // Imprime la memoria disponible
    printf("Memoria swap libre: %ld KB\n", mem.swap_free);                                   // Imprime la memoria swap libre
    printf("Memoria swap usada: %ld KB\n", mem.swap_used);                                   // Imprime la memoria swap usada
}
