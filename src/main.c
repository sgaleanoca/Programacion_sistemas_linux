#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "logger.h"
#include "sensor_mock.h"

// Variable global para controlar el bucle principal.
// 'volatile' evita que el compilador la optimice, y 'sig_atomic_t' asegura
// que la escritura sea atómica (indivisible) durante una señal.
static volatile sig_atomic_t keep_running = 1;

// Manejador de la señal SIGTERM
void sigterm_handler(int signum) {
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    int interval = 5; // Intervalo por defecto: 5 segundos

    // Parseo simple de argumentos para --interval
    if (argc == 3 && strcmp(argv[1], "--interval") == 0) {
        interval = atoi(argv[2]);
        if (interval <= 0) {
            fprintf(stderr, "El intervalo debe ser un número positivo.\n");
            return 1;
        }
    }

    // Registrar el manejador para la señal SIGTERM
    signal(SIGTERM, sigterm_handler);

    if (init_logger() != 0) {
        return 1; // Salir si no se pudo crear el log
    }
    
    initialize_sensor();

    printf("Iniciando servicio de sensor. Log en /tmp/sensor_log.txt o /var/tmp/sensor_log.txt. Presione Ctrl+C para salir (si se ejecuta manualmente).\n");

    // Bucle principal
    while (keep_running) {
        float value = get_sensor_value();
        log_data(value);
        sleep(interval);
    }

    printf("\nSeñal recibida. Cerrando de forma ordenada...\n");
    close_logger()

    return 0;
}