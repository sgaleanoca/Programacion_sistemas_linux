CC = gcc
CFLAGS = -Wall -O2
SRC = src/main.c src/logger.c src/sensor_mock.c
OBJ = $(SRC:.c=.o)
BIN = mock-sensor

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
UNITDIR = /etc/systemd/system

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

install: $(BIN)
	mkdir -p $(BINDIR)
	install -m 755 $(BIN) $(BINDIR)/
	install -m 644 systemd/assignment-sensor.service $(UNITDIR)/

uninstall:
	rm -f $(BINDIR)/$(BIN)
	rm -f $(UNITDIR)/assignment-sensor.service

.PHONY: all clean install uninstall
