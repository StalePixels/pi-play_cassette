CC = gcc

CLIBS = -lasound -lm
COPTS = -g 
CFLAGS = -Iinc $(CLIBS) $(COPTS)

DEPS = $(wildcard *.h)
OBJS = shutdown.o usage.o \
		tzx_file.o tzx_load.o \
		tzx-pi.o 

.PHONY: clean tzx-pi

default: tzx-pi

%.o: %.c %(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: tzx-pi

tzx-pi: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) 

test-tzx:
	@clear
	@$(MAKE) tzx-pi
	@echo "\nSTARTING TEST" && ./tzx-pi ../Gauntlet\ -\ Side\ 1.tzx

clean:
	rm -f *.o *~ core