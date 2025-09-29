# Compilador
CC = gcc
# Flags de compilación: -Wall (todos los warnings), -g (info de debug)
CFLAGS = -Wall -g

# Directorios
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/assignment-sensor

# Archivos fuente
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/logger.c $(SRCDIR)/sensor_mock.c
# Archivos objeto (se crearán en el directorio de build)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

# Regla por defecto: se ejecuta al escribir 'make'
all: $(TARGET)

# Regla para enlazar el ejecutable final
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

# Regla para compilar los archivos .c en .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Regla para limpiar el proyecto
clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean