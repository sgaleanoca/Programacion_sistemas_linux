#include "sensor_mock.h"
#include <stdlib.h>
#include <stdio.h>

// Genera un valor ficticio de "sensor"
int mock_sensor(int counter, const char *device_path) {
    switch (counter % 3) {
        case 0: // random del dispositivo especificado
        {
            unsigned int r;
            FILE *f = fopen(device_path, "rb");
            if (f) {
                fread(&r, sizeof(r), 1, f);
                fclose(f);
            } else {
                r = rand();
            }
            return (int)(r % 1000); // 0-999
        }
        case 1: // contador simple
            return counter;
        case 2: // temperatura ficticia (20-30)
            return 20 + rand() % 11;
    }
    return 0;
}
