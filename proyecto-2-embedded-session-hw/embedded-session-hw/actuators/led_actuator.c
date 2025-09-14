#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "actuator.h"

/* Estructura interna para manejar el estado del LED */
typedef struct {
    bool is_on;
} LedParams;

/* Implementaciones específicas */
static void led_activate(void *params) {
    LedParams *led = (LedParams *) params;
    led->is_on = true;
    printf("[LED] Encendido\n");
}

static void led_deactivate(void *params) {
    LedParams *led = (LedParams *) params;
    led->is_on = false;
    printf("[LED] Apagado\n");
}

static bool led_status(void *params) {
    LedParams *led = (LedParams *) params;
    return led->is_on;
}

/* Función de fábrica: devuelve un Actuator configurado como LED */
Actuator create_led_actuator(void) {
    LedParams *params = (LedParams *) malloc(sizeof(LedParams));
    params->is_on = false;

    Actuator led = {
        .params = params,
        .activate = led_activate,
        .deactivate = led_deactivate,
        .status = led_status
    };

    return led;
}
