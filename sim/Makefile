# From the repo root:
#   make -C sim
#   ./sim/retrodiode-ui
#   make -C sim test

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I.
SDL_CFLAGS = $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2)

CORE = engine/signal.c chips/max7219.c chips/avr128db28.c board.c

TEST_BINS = tests/test_max7219 tests/test_avr128db28 tests/test_board

retrodiode-ui: main.c $(CORE)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main.c $(CORE) $(LDLIBS)

tests/test_max7219: tests/test_max7219.c chips/max7219.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_max7219.c chips/max7219.c engine/signal.c

tests/test_avr128db28: tests/test_avr128db28.c chips/avr128db28.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_avr128db28.c chips/avr128db28.c engine/signal.c

tests/test_board: tests/test_board.c $(CORE)
	$(CC) $(CFLAGS) -o $@ tests/test_board.c $(CORE)

test: $(TEST_BINS)
	./tests/test_max7219
	./tests/test_avr128db28
	./tests/test_board

clean:
	rm -f retrodiode-ui test_chips $(TEST_BINS)
