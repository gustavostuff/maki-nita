CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image)
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_image)

TARGET = acetate-sim

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main.c $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
