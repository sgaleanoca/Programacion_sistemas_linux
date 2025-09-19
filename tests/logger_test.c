// tests/logger_test.c
#include <stdio.h>
#include "logger.h"

int main() {
    if (logger_init("/tmp/test.log") != 0) {
        fprintf(stderr, "Fallo init logger\n");
        return 1;
    }
    logger_log("2025-09-19T18:30:00Z", 123);
    logger_close();
    printf("Logger test OK\n");
    return 0;
}
