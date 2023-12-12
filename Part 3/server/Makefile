CFLAGS=-Wall -Wextra -Og -g

SERVER_SRC=server.c queue.c
SERVER_OBJ=$(SERVER_SRC:.c=.o)

CLIENT_SRC=client.c queue.c
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)

all: server client

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f $(SERVER_OBJ) server $(CLIENT_OBJ) client
