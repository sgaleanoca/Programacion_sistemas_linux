#include "sensor_mock.h"
#include <stdlib.h>
#include <time.h>

// Inicializa el generador de números aleatorios una sola vez
void initialize_sensor() {
    srand(time(NULL));
}

float get_sensor_value() {
    // Genera un número flotante entre 20.0 y 30.0
    return 20.0f + (float)rand() / ((float)RAND_MAX / 10.0f);
}