CC=gcc
CFLAGS=-Wall -Wextra -O2

SRC=src/main.c src/logger.c src/sensor_mock.c
OBJ=$(SRC:.c=.o)
BIN=assignment-sensor

TESTS=tests/logger_test tests/sensor_mock_test

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

tests/logger_test: src/logger.c src/logger.h tests/logger_test.c
	$(CC) $(CFLAGS) -o $@ $^

tests/sensor_mock_test: src/sensor_mock.c src/sensor_mock.h tests/sensor_mock_test.c
	$(CC) $(CFLAGS) -o $@ $^

test: $(TESTS)
	./tests/logger_test
	./tests/sensor_mock_test

install: $(BIN)
	sudo cp $(BIN) /usr/local/bin/sensor_logger
	sudo cp systemd/assignment-sensor.service /etc/systemd/system/
	sudo systemctl daemon-reload

uninstall:
	sudo systemctl stop assignment-sensor || true
	sudo systemctl disable assignment-sensor || true
	sudo rm -f /usr/local/bin/sensor_logger
	sudo rm -f /etc/systemd/system/assignment-sensor.service
	sudo systemctl daemon-reload

clean:
	rm -f $(BIN) src/*.o tests/*_test
