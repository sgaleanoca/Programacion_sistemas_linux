#ifndef SENSOR_H
#define SENSOR_H

/* Inicializa el sensor (puede ser aleatorio o desde un archivo) */
void sensor_init(void);

/* Lee un valor del sensor y lo devuelve como double */
double sensor_read(void);

#endif /* SENSOR_H */
