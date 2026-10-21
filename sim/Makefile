# From the repo root:
#   make -C sim
#   ./sim/retrodiode-ui

CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2)

retrodiode-ui: main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDLIBS)

clean:
	rm -f retrodiode-ui
