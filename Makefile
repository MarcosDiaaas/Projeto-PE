CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include

SRC_DIR = src
OBJ_DIR = obj
TARGET = ufabc_zipz

# Garante que os arquivos da pasta 'testes' NÃO sejam compilados junto com o executável final
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ATENÇÃO: A adição do -lm no final da linha é crucial por causa da matemática na entropia
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET).exe