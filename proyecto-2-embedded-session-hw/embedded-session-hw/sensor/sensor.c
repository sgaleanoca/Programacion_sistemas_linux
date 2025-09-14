#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sensor.h"

/* Inicialización del sensor */
void sensor_init(void) {
    /* Semilla para números aleatorios */
    srand((unsigned int) time(NULL));
    printf("[SENSOR] Inicializado correctamente.\n");
}

/* Lectura del sensor */
double sensor_read(void) {
    /* Retorna un valor entre 0.0 y 100.0 */
    double value = (double)(rand() % 101);
    return value;
}
