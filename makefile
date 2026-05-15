# diretórios
INC_DIR 		:= include
SRC_DIR			:= src
BIN_DIR			:= bin
OBJ_DIR			:= src/obj

# flags 
CC				:= gcc
CFLAGS 			:= -Wall -Wextra -Wpedantic -Wno-packed-bitfield-compat -I$(INC_DIR)
LDFLAGS 		:= -lncurses

# arquivos fonte
SRC_CLIENT		:= client.c interface.c
SRC_SERVER		:= server.c game.c ghosts.c pacman.c pellets.c
SRC_COMMON		:= utils.c kermit.c

# arquivos objetos
OBJ_CLIENT		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_CLIENT)))
OBJ_SERVER		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_SERVER)))
OBJ_COMMON		:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_COMMON)))

# executáveis 
TARGET_CLIENT 	:= $(BIN_DIR)/client
TARGET_SERVER	:= $(BIN_DIR)/server


# ==============
.PHONY: all clean client server

all: $(TARGET_CLIENT) $(TARGET_SERVER)

# caminho dos .c em todos os diretórios
vpath %.c $(shell find $(SRC_DIR) -type d)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_CLIENT): $(OBJ_CLIENT) $(OBJ_COMMON)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): $(OBJ_SERVER) $(OBJ_COMMON)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

client: $(TARGET_CLIENT)
	./$<

server: $(TARGET_SERVER)
	./$<

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)