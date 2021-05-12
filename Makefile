
# CC=gcc
# CFLAGS=-Iinc
# DEPS = tzx-pi.h
# OBJ = tzx-pi.o 
# CLIBS = -lasound -ldl -lm

# %.o: src/%.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# tzx-pi: src/$(OBJ)
# 	$(CC) -o $@ $^ $(CFLAGS) $(CLIBS)

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean
