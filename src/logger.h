#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Abre el archivo de log. Devuelve 0 si tiene éxito, -1 si falla.
int init_logger();

// Escribe un registro en el log
void log_data(float value);

// Cierra el archivo de log
void close_logger();

#endif // LOGGER_H