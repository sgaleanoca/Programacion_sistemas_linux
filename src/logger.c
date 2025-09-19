#include "logger.h"
#include <time.h>
#include <stdlib.h>

FILE* open_log_file() {
    FILE *f = fopen("/tmp/mock_sensor.log", "a");
    if (!f) {
        f = fopen("/var/tmp/mock_sensor.log", "a"); // fallback
    }
    return f;
}

void log_value(FILE *f, int value) {
    if (!f) return;
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
    fprintf(f, "%s Value=%d\n", buf, value);
    fflush(f);
}

void close_log_file(FILE *f) {
    if (f) fclose(f);
}
