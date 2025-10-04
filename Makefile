all:
	cc src/dy.c -o dy $(shell pkg-config --libs --cflags python3-embed) $(shell pkg-config --libs --cflags m)

release:
	cc src/dy.c -o dy -O3 $(shell pkg-config --libs --cflags python3-embed) $(shell pkg-config --libs --cflags m)
