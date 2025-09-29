CC=gcc
CFLAGS=-Wall -Wextra -std=c11

SRC_SENSOR = sensor/sensor.c
SRC_ACTUATORS = actuators/led_actuator.c actuators/buzzer_actuator.c
SRC_CTL = controller/ctl.c

OBJ = $(SRC_SENSOR) $(SRC_ACTUATORS) $(SRC_CTL)

all: ctl64 ctl32

ctl64:
	$(CC) $(CFLAGS) -m64 -o ctl64 $(OBJ)

ctl32:
	$(CC) $(CFLAGS) -m32 -o ctl32 $(OBJ)

clean:
	rm -f ctl64 ctl32

