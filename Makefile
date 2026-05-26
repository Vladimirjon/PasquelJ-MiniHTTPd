CC=gcc
CFLAGS=-Wall -Wextra -O2 -Iinclude

SRC=src/main.c src/server.c src/http.c src/files.c src/mime.c
OBJ=$(SRC:.c=.o)
BIN=minihttpd

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)