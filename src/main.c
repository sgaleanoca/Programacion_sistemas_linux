#include "sensor_mock.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static int keep_running = 1;

void handle_sigterm(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    int interval = 5; // default 5s
    if (argc > 1) {
        interval = atoi(argv[1]);
        if (interval <= 0) interval = 5;
    }

    FILE *log = open_log_file();
    if (!log) {
        fprintf(stderr, "Error: cannot open log file\n");
        return 1;
    }

    signal(SIGTERM, handle_sigterm);

    while (keep_running) {
        int value = read_mock_sensor();
        log_value(log, value);
        sleep(interval);
    }

    close_log_file(log);
    return 0;
}

