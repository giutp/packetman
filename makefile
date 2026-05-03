# Diretórios
SRC_DIR     := src
INC_DIR     := include
BIN_DIR     := bin

# Flags
CC          := gcc
CFLAGS      := -Wall -Wextra -Wpedantic -g -I$(INC_DIR)
LDFLAGS     := -lncurses

# Alvos finais (Binários)
TARGET_CLIENT := $(BIN_DIR)/client
TARGET_SERVER := $(BIN_DIR)/server

# Arquivos compartilhados (Rede)
COMMON_SRC  := $(SRC_DIR)/network/kermit.c

# Arquivos do Cliente (Interface + Rede)
# Corrigido: client.c está solto em src/
CLIENT_SRC  := $(SRC_DIR)/client.c $(SRC_DIR)/core/interface.c \
               $(COMMON_SRC)

# Arquivos do Servidor (Lógica do Jogo + Rede)
# Corrigido: server.c está solto em src/ (main.c ignorada)
SERVER_SRC  := $(SRC_DIR)/server.c $(SRC_DIR)/core/game.c \
               $(SRC_DIR)/core/ghosts.c $(SRC_DIR)/core/pacman.c \
               $(SRC_DIR)/core/pellets.c \
               $(COMMON_SRC)

# Troca a extensão .c por .o
CLIENT_OBJS := $(CLIENT_SRC:.c=.o)
SERVER_OBJS := $(SERVER_SRC:.c=.o)

# ==============================================
.PHONY: all clean run_client run_server rebuild

all: $(TARGET_CLIENT) $(TARGET_SERVER)

# Compila o executável do Cliente
$(TARGET_CLIENT): $(CLIENT_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compila o executável do Servidor
$(TARGET_SERVER): $(SERVER_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Regra genérica para compilar os .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza: apaga os .o de dentro do src/, src/network/, src/core/ e executáveis
clean:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/network/*.o $(SRC_DIR)/core/*.o $(TARGET_CLIENT) $(TARGET_SERVER)

run_client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

run_server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

rebuild: clean all