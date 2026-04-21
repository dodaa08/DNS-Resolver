CC     = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

SRC = src/main.c src/dns.c
BIN = dns-resolver

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)