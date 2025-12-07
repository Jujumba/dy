CC=cc
FSANITIZE=-fsanitize=undefined -fsanitize=address
CFLAGS= -Wall -Wno-char-subscripts
LIBS=$(shell pkg-config --libs --cflags python3-embed) -lm

debug:
	$(CC) src/dy.c -o dy -g $(CFLAGS) $(FSANITIZE) $(LIBS)

release:
	$(CC) src/dy.c -o dy -O3 $(CFLAGS) $(FSANITIZE) $(LIBS)

all:
	debug
