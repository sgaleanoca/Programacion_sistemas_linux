#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sensor.h"

/* Variables globales para el estado del sensor */
static sensor_mode_t current_mode = SENSOR_MODE_RANDOM;
static char csv_filename[256] = {0};
static int csv_line_count = 0;
static int csv_current_line = 0;
static double *csv_values = NULL;

/* Función auxiliar para contar líneas en el CSV */
static int count_csv_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    fclose(file);
    
    /* Restar 1 por el header */
    return (count > 0) ? count - 1 : 0;
}

/* Función auxiliar para cargar valores del CSV */
static bool load_csv_values(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("[SENSOR] Error: No se pudo abrir el archivo %s\n", filename);
        return false;
    }
    
    /* Contar líneas de datos */
    csv_line_count = count_csv_lines(filename);
    if (csv_line_count <= 0) {
        printf("[SENSOR] Error: El archivo CSV no contiene datos válidos\n");
        fclose(file);
        return false;
    }
    
    /* Asignar memoria para los valores */
    csv_values = (double *)malloc(csv_line_count * sizeof(double));
    if (!csv_values) {
        printf("[SENSOR] Error: No se pudo asignar memoria\n");
        fclose(file);
        return false;
    }
    
    /* Saltar header */
    char line[256];
    fgets(line, sizeof(line), file);
    
    /* Leer valores */
    int index = 0;
    while (fgets(line, sizeof(line), file) && index < csv_line_count) {
        char *comma = strchr(line, ',');
        if (comma) {
            csv_values[index] = atof(comma + 1);
            index++;
        }
    }
    
    fclose(file);
    csv_current_line = 0;
    printf("[SENSOR] Cargados %d valores desde %s\n", csv_line_count, filename);
    return true;
}

/* Inicialización del sensor en modo aleatorio */
void sensor_init(void) {
    current_mode = SENSOR_MODE_RANDOM;
    srand((unsigned int) time(NULL));
    printf("[SENSOR] Inicializado en modo aleatorio.\n");
}

/* Inicialización del sensor con archivo CSV */
void sensor_init_csv(const char *csv_file_path) {
    current_mode = SENSOR_MODE_CSV;
    strncpy(csv_filename, csv_file_path, sizeof(csv_filename) - 1);
    csv_filename[sizeof(csv_filename) - 1] = '\0';
    
    if (load_csv_values(csv_file_path)) {
        printf("[SENSOR] Inicializado en modo CSV con archivo: %s\n", csv_file_path);
    } else {
        printf("[SENSOR] Error al cargar CSV, cambiando a modo aleatorio\n");
        sensor_init();
    }
}

/* Lectura del sensor */
double sensor_read(void) {
    if (current_mode == SENSOR_MODE_RANDOM) {
        /* Retorna un valor aleatorio entre 0.0 y 100.0 */
        return (double)(rand() % 101);
    } else if (current_mode == SENSOR_MODE_CSV && csv_values) {
        if (csv_current_line < csv_line_count) {
            double value = csv_values[csv_current_line];
            csv_current_line++;
            return value;
        } else {
            /* Si llegamos al final, reiniciamos */
            csv_current_line = 0;
            return csv_values[0];
        }
    }
    
    /* Fallback a valor aleatorio */
    return (double)(rand() % 101);
}

/* Obtiene el modo actual del sensor */
sensor_mode_t sensor_get_mode(void) {
    return current_mode;
}

/* Verifica si el sensor está en modo CSV y si hay más datos */
bool sensor_has_more_data(void) {
    if (current_mode == SENSOR_MODE_CSV && csv_values) {
        return csv_current_line < csv_line_count;
    }
    return true; /* En modo aleatorio siempre hay "más datos" */
}

/* Reinicia el replay del archivo CSV */
void sensor_reset_csv(void) {
    if (current_mode == SENSOR_MODE_CSV) {
        csv_current_line = 0;
        printf("[SENSOR] Replay del CSV reiniciado\n");
    }
}
