#include "logger.h"
#include <time.h>
#include <string.h>

static FILE *logfile = NULL;

int init_logger() {
    // Intenta abrir en /tmp primero
    logfile = fopen("/tmp/sensor_log.txt", "a");
    if (logfile == NULL) {
        // Si falla, intenta en /var/tmp como fallback
        logfile = fopen("/var/tmp/sensor_log.txt", "a");
    }

    if (logfile == NULL) {
        perror("Error: No se pudo abrir el archivo de log en /tmp ni en /var/tmp");
        return -1;
    }
    return 0;
}

void log_data(float value) {
    if (logfile == NULL) return;

    char timestamp[30];
    time_t now = time(NULL);
    // Formatea el tiempo a ISO-8601 UTC (con la 'Z')
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

    fprintf(logfile, "[%s] Sensor Value: %.2f\n", timestamp, value);
    fflush(logfile); // Asegura que se escriba inmediatamente en el disco
}

void close_logger() {
    if (logfile != NULL) {
        fclose(logfile);
        logfile = NULL;
    }
}