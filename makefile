# ─────────────────────────────────────────────
# Makefile — juego en C con Allegro 5
#
# CÓMO USARLO:
#   make        → compilar
#   make run    → compilar y ejecutar
#   make clean  → borrar obj/ y el ejecutable
#
# CÓMO AGREGAR UN MÓDULO NUEVO:
#   1. Crea src/tu_modulo.c
#   2. Agrégalo a SRCS abajo
#   3. Corre make de nuevo
# ─────────────────────────────────────────────

TARGET  = juego
CC      = gcc
CFLAGS  = -Wall -Wextra -g

# Directorios
SRC_DIR = src
OBJ_DIR = obj

# ─────────────────────────────────────────────
# ARCHIVOS FUENTE
# Cambia esta lista si agregas o eliminas módulos.
# ─────────────────────────────────────────────
SRCS = main.c     \
       game.c     \
       input.c    \
       entities.c \
       update.c   \
       renderer.c \
       ui.c       \
       assets.c

# Construir rutas completas: src/main.c, src/game.c, ...
SRCS_FULL = $(addprefix $(SRC_DIR)/, $(SRCS))

# Construir rutas de objetos: obj/main.o, obj/game.o, ...
# Primero agrega prefijo src/, luego reemplaza src/*.c por obj/*.o
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_FULL))

# ─────────────────────────────────────────────
# LIBRERÍAS DE ALLEGRO
# Agrega o quita addons según los que uses.
# ─────────────────────────────────────────────
LIBS = $(shell pkg-config --libs \
         allegro-5              \
         allegro_image-5        \
         allegro_font-5         \
         allegro_ttf-5          \
         allegro_audio-5        \
         allegro_acodec-5       \
         allegro_primitives-5)

INCS = $(shell pkg-config --cflags allegro-5) -I$(SRC_DIR)

# ─────────────────────────────────────────────
# REGLAS
# ─────────────────────────────────────────────

# Regla principal: enlazar todos los .o en el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Regla genérica: compilar src/*.c → obj/*.o
# El | obj/ significa "asegúrate de que obj/ exista primero"
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/commons.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# Crear la carpeta obj/ si no existe
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: run clean
