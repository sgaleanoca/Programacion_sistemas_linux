#ifndef MEMORY_H
#define MEMORY_H

// Estructura para guardar info de la memoria
typedef struct {
    long total;                                     // Memoria total en KB
    long free;                                      // Memoria libre en KB
    long available;                                 // Memoria disponible para el sistema
    long used;                                      // Memoria física usada
    long swap_total;                                // Memoria swap total
    long swap_free;                                 // Memoria swap libre
    long swap_used;                                 // Memoria swap usada (virtual)
} MemoryInfo;                                       // Estructura para guardar info de la memoria

// Funciones públicas
MemoryInfo get_memory_info();                       // Obtiene la info de la memoria
void print_memory_info(MemoryInfo mem);             // Imprime la info de la memoria

#endif
