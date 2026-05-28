# diretórios
INC_DIR 		:= include
SRC_DIR			:= src
OBJ_DIR			:= src/obj
DOW_DIR			:= assets/download

# flags 
CC				:= gcc
CFLAGS 			:= -Wall -Wextra -Wpedantic -Wno-packed-bitfield-compat -g -I$(INC_DIR)
LDFLAGS 		:= -lncurses

# arquivos fonte
SRC_CLIENT		:= client.c interface.c
# SRC_CLIENT		:= test-client.c interface.c
SRC_SERVER		:= server.c game.c ghosts.c pacman.c pellets.c
# SRC_SERVER		:= test-server.c game.c ghosts.c pacman.c pellets.c
SRC_COMMON		:= utils.c kermit.c

# arquivos objetos
OBJ_CLIENT		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_CLIENT)))
OBJ_SERVER		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_SERVER)))
OBJ_COMMON		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_COMMON)))

# executáveis 
TARGET_CLIENT 	:= client
TARGET_SERVER	:= server


# ==============
.PHONY: all clean client server

# caminho dos .c em todos os diretórios
vpath %.c $(shell find $(SRC_DIR) -type d)

all: client server

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

client: $(OBJ_CLIENT) $(OBJ_COMMON)
	@mkdir -p $(DOW_DIR)
	$(CC) $^ -o $(TARGET_CLIENT) $(LDFLAGS)

server: $(OBJ_SERVER) $(OBJ_COMMON)
	$(CC) $^ -o $(TARGET_SERVER)

clean:
	@rm -rf $(OBJ_DIR) $(DOW_DIR) $(TARGET_CLIENT) $(TARGET_SERVER)