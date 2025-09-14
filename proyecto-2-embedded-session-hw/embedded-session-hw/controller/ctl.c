#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>   /* usleep */
#include "../sensor/sensor.h"
#include "../actuators/actuator.h"

/* Prototipos de las fábricas de actuadores */
Actuator create_led_actuator(void);
Actuator create_buzzer_actuator(void);

/* Tiempo monotónico (segundos con decimales) */
static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void) {
    /* Umbral fijo para este ejemplo */
    const double THRESHOLD = 50.0;

    /* Inicializar sensor */
    sensor_init();

    /* Crear actuadores */
    Actuator led = create_led_actuator();
    Actuator buzzer = create_buzzer_actuator();

    /* Variables para tiempos de apagado diferido */
    double led_off_time = 0.0;
    double buzzer_off_time = 0.0;

    printf("=== CONTROLADOR INICIADO ===\n");

    /* Bucle infinito de muestreo cada 100 ms */
    while (1) {
        double t = now();
        double val = sensor_read();

        if (val >= THRESHOLD) {
            /* Si supera el umbral, encender de inmediato y cancelar apagados */
            led.activate(led.params);
            buzzer.activate(buzzer.params);
            led_off_time = 0.0;
            buzzer_off_time = 0.0;
        } else {
            /* Si no supera el umbral, programar apagados diferidos */
            if (buzzer_off_time == 0.0) buzzer_off_time = t + 1.0; /* 1 s */
            if (led_off_time == 0.0) led_off_time = t + 5.0;       /* 5 s */
        }

        /* Revisar si ya se cumplió el tiempo para apagar */
        if (buzzer_off_time > 0.0 && t >= buzzer_off_time) {
            buzzer.deactivate(buzzer.params);
            buzzer_off_time = 0.0;
        }

        if (led_off_time > 0.0 && t >= led_off_time) {
            led.deactivate(led.params);
            led_off_time = 0.0;
        }

        /* Log de estado */
        printf("[t=%.2f] Sensor=%.2f | LED=%s | BUZZER=%s\n",
               t, val,
               led.status(led.params) ? "ON" : "OFF",
               buzzer.status(buzzer.params) ? "ON" : "OFF");

        /* Esperar 100 ms */
        struct timespec delay = {0, 100000000}; /* 100ms = 100,000,000 ns */
        nanosleep(&delay, NULL);
    }

    return 0;
}
