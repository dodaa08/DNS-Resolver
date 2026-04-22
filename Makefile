CC     = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

SRC = src/main.c src/dns.c src/cache.c
BIN = dns-resolver

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

test:
	$(CC) $(CFLAGS) -o dns-test tests/test.c src/dns.c src/cache.c
	./dns-test

clean:
	rm -f $(BIN) dns-test