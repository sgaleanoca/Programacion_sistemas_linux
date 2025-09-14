#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>

/* Tipos de modo de sensor */
typedef enum {
    SENSOR_MODE_RANDOM,    /* Valores aleatorios */
    SENSOR_MODE_CSV        /* Replay desde archivo CSV */
} sensor_mode_t;

/* Inicializa el sensor en modo aleatorio */
extern void sensor_init(void);

/* Inicializa el sensor con un archivo CSV específico */
extern void sensor_init_csv(const char *csv_file);

/* Lee un valor del sensor y lo devuelve como double */
extern double sensor_read(void);

/* Obtiene el modo actual del sensor */
extern sensor_mode_t sensor_get_mode(void);

/* Verifica si el sensor está en modo CSV y si hay más datos */
extern bool sensor_has_more_data(void);

/* Reinicia el replay del archivo CSV */
extern void sensor_reset_csv(void);

#endif /* SENSOR_H */
