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
CLIENT_SRC  := $(SRC_DIR)/client.c $(SRC_DIR)/core/interface.c \
               $(COMMON_SRC)

# Arquivos do Servidor (Lógica do Jogo + Rede)
SERVER_SRC  := $(SRC_DIR)/server.c $(SRC_DIR)/core/game.c \
               $(SRC_DIR)/core/ghosts.c $(SRC_DIR)/core/pacman.c \
               $(SRC_DIR)/core/pellets.c \
               $(COMMON_SRC)


CLIENT_OBJS := $(CLIENT_SRC:.c=.o)
SERVER_OBJS := $(SERVER_SRC:.c=.o)

# =====================================================
# Ações do make
.PHONY: all clean run_client run_server rebuild

all: $(TARGET_CLIENT) $(TARGET_SERVER)

# Compila o executável do Cliente
$(TARGET_CLIENT): $(CLIENT_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compila o executável do Servidor
$(TARGET_SERVER): $(SERVER_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compila os .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa os .o e os executáveis
clean:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/network/*.o $(SRC_DIR)/core/*.o $(TARGET_CLIENT) $(TARGET_SERVER)

run_client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

run_server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

rebuild: clean all