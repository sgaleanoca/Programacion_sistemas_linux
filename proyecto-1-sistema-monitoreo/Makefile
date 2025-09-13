CC = gcc 
CFLAGS = -Wall -Wextra -Iinclude 
SRC = src/main.c src/cpu.c src/memory.c
OBJ = $(SRC:.c=.o) 
TARGET = system_info 

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) $(OBJ) -o $@ 

clean:
	rm -f $(OBJ) $(TARGET)  
