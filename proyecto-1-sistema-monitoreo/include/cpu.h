#ifndef CPU_H
#define CPU_H

// Estructura para guardar info del CPU
typedef struct {
    char model_name[128];                                   // Nombre del procesador
    int cores;                                              // Cantidad de cores
} CPUInfo;                                                  // Estructura para guardar info del CPU

// Funciones públicas
CPUInfo get_cpu_info();                                     // Obtiene la info del CPU
void print_cpu_info(CPUInfo cpu);                           // Imprime la info del CPU
void get_cpu_load_per_core(float *loads, int cores);        // Obtiene la carga del CPU por core

#endif
