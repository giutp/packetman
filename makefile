# Flags padrões
CC 		:= gcc
CFLAGS 	:= -Wall -Wextra -Wpedantic -g
LDFLAGS := -lncurses

# Todos os alvos
TARGET_CLIENT := client
TARGET_SERVER := server

# Arquivos compartilhados
COMMON := kermit.o

# .o dos executaveis
CLIENT_OBJS := client.o interface.o $(COMMON)
SERVER_OBJS := server.o $(COMMON)

# ==============================================
.PHONY: all clean run rebuild

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(CLIENT_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_SERVER): $(SERVER_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET_CLIENT) $(TARGET_SERVER)

run_client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

run_client: $(TARGET_SERVER)
	./$(TARGET_SERVER)

rebuild: clean all