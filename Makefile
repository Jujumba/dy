CC=cc
FSANITIZE=-fsanitize=undefined -fsanitize=address
PYTHON3_EMBED=$(shell pkg-config --libs --cflags python3-embed)
LIBM=$(shell pkg-config --libs --cflags m)

debug:
	$(CC) src/dy.c -o dy -g -Wall $(FSANITIZE) $(PYTHON3_EMBED) $(LIBM)

release:
	$(CC) src/dy.c -o dy -O3 -Wall $(FSANITIZE) $(PYTHON3_EMBED) $(LIBM)

all:
	debug
