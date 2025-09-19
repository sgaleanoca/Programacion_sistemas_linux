#include "logger.h"
#include <stdio.h>

void log_sample(FILE *logfile, const char *timestamp, int value) {
    fprintf(logfile, "%s value=%d\n", timestamp, value);
}
