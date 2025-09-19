#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

FILE* open_log_file();
void log_value(FILE *f, int value);
void close_log_file(FILE *f);

#endif
