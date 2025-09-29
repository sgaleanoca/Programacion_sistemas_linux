#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.h"
#include "memory.h"

int main() {
    CPUInfo cpu = get_cpu_info();                                   // Obtiene la info del CPU
    MemoryInfo mem;                                                 // Estructura para guardar info de la memoria

    float cpu_loads[cpu.cores];                                     // Array para guardar la carga del CPU por core

    while (1) {
        system("clear");                                            // Limpia la pantalla estilo terminal

        // Memoria
        mem = get_memory_info();                                    // Obtiene la info de la memoria
        print_memory_info(mem);                                     // Imprime la info de la memoria

        // CPU
        print_cpu_info(cpu);                                        // Imprime la info del CPU
        get_cpu_load_per_core(cpu_loads, cpu.cores);                // Obtiene la carga del CPU por core
        for (int i = 0; i < cpu.cores; i++) {                       // Imprime la carga del CPU por core
            printf("Core %d: %.2f%%\n", i, cpu_loads[i]);           // Imprime la carga del CPU por core
        }

        sleep(2);                                                   // Espera 2 segundos
    }

    return 0;                                                       // Retorna 0
}
