CC      ?= gcc
CFLAGS  ?= -O2 -g
CFLAGS  += -Wall -Wextra -std=c99 -Iinclude
LDLIBS  := -lspectrum -lasound -lm

BIN     := nextpi-play_cassette
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:src/%.c=build/%.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	@mkdir -p $@

clean:
	rm -rf build $(BIN)

.PHONY: all clean
