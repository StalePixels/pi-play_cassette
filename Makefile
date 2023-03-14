CC = gcc

CLIBS = -lasound -lm
COPTS = -g 
CFLAGS = -Iinc $(CLIBS) $(COPTS)

DEPS = $(wildcard *.h)
OBJS = shutdown.o usage.o \
		tzx_file.o tzx_load.o \
		tzx_get_block_type.o tzx_display_checksum.o \
		tzx_ticks_to_samples.o tzx_toggle_amplitude.o \
		tzx_play_audio.o tzx_pause_audio.o tzx_finish_audio.o \
		tzx_print_string.o \
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

jack:
	make clean && \
	scp -r /u/GIT/tzx-pi/src/* dietpi@jack:/u/GIT/tzx-pi/src/ && \
	ssh dietpi@jack "cd /u/GIT/tzx-pi/ && make"

clean:
	rm -f *.o *~ core tzx-pi