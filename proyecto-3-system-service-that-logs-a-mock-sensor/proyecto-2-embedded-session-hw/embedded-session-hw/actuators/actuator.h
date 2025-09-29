#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdbool.h>

/* Definición de una interfaz polimórfica para actuadores */
typedef struct {
    void *params;                                /* Datos específicos del actuador */
    void (*activate)(void *params);              /* Función para activar */
    void (*deactivate)(void *params);            /* Función para desactivar */
    bool (*status)(void *params);                /* Estado actual (ON/OFF) */
} Actuator;

#endif /* ACTUATOR_H */
