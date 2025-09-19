#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "logger.h"
#include "sensor_mock.h"

static volatile int keep_running = 1;

void handle_sigterm(int sig) {
    (void)sig; // evitar warning
    keep_running = 0;
}

void print_usage(const char *progname) {
    printf("Uso: %s [--interval <seconds>] [--logfile <path>] [--device <path>]\n", progname);
    printf("Opciones:\n");
    printf("  --interval <seconds>  Intervalo entre lecturas (default: 5)\n");
    printf("  --logfile <path>      Archivo de log (default: /tmp/sensor.log)\n");
    printf("  --device <path>       Dispositivo a leer (default: /dev/urandom)\n");
    printf("  --help               Mostrar esta ayuda\n");
}

int main(int argc, char *argv[]) {
    int interval = 5; // default 5s
    char *logfile_path = NULL;
    char *device_path = "/dev/urandom"; // default device
    FILE *logfile = NULL;

    // parsear argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
            interval = atoi(argv[i + 1]);
            i++; // saltar el siguiente argumento
        } else if (strcmp(argv[i], "--logfile") == 0 && i + 1 < argc) {
            logfile_path = argv[i + 1];
            i++; // saltar el siguiente argumento
        } else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            device_path = argv[i + 1];
            i++; // saltar el siguiente argumento
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Error: argumento desconocido '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // abrir log
    if (logfile_path) {
        logfile = fopen(logfile_path, "a");
    } else {
        // fallback: primero /tmp, luego /var/tmp
        logfile = fopen("/tmp/sensor.log", "a");
        if (!logfile) {
            logfile = fopen("/var/tmp/sensor.log", "a");
        }
    }
    
    if (!logfile) {
        fprintf(stderr, "Error: no se pudo abrir archivo de log: %s\n", strerror(errno));
        return 1;
    }

    // capturar señal SIGTERM
    signal(SIGTERM, handle_sigterm);

    int counter = 0;
    while (keep_running) {
        // obtener timestamp ISO-8601
        char ts[32];
        time_t now = time(NULL);
        struct tm *utc = gmtime(&now);
        strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", utc);

        // leer valor de sensor ficticio
        int value = mock_sensor(counter, device_path);

        // escribir en log
        log_sample(logfile, ts, value);

        counter++;
        fflush(logfile);
        sleep(interval);
    }

    fclose(logfile);
    return 0;
}
