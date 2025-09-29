#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "actuator.h"

/* Estructura interna para manejar el estado del buzzer */
typedef struct {
    bool is_on;
} BuzzerParams;

/* Implementaciones específicas */
static void buzzer_activate(void *params) {
    BuzzerParams *bz = (BuzzerParams *) params;
    bz->is_on = true;
    printf("[BUZZER] Activado\n");
}

static void buzzer_deactivate(void *params) {
    BuzzerParams *bz = (BuzzerParams *) params;
    bz->is_on = false;
    printf("[BUZZER] Desactivado\n");
}

static bool buzzer_status(void *params) {
    BuzzerParams *bz = (BuzzerParams *) params;
    return bz->is_on;
}

/* Función de fábrica: devuelve un Actuator configurado como buzzer */
Actuator create_buzzer_actuator(void) {
    BuzzerParams *params = (BuzzerParams *) malloc(sizeof(BuzzerParams));
    params->is_on = false;

    Actuator buzzer = {
        .params = params,
        .activate = buzzer_activate,
        .deactivate = buzzer_deactivate,
        .status = buzzer_status
    };

    return buzzer;
}
