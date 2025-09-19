#include "sensor_mock.h"
#include <stdlib.h>
#include <stdio.h>

int read_mock_sensor() {
    // Ejemplo simple: valor aleatorio 0-999
    return rand() % 1000;
}

