# From the repo root:
#   make -C sim
#   ./sim/retrodiode-ui
#   make -C sim test

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -I../fw
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image)
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_image)

CORE = engine/signal.c chips/max7219.c chips/avr128db28.c board.c
GAME = ../fw/frame.c ../fw/input.c ../fw/game.c \
       ../fw/entity/base_entity.c ../fw/entity/player.c \
       ../fw/entity/bullet.c ../fw/entity/rock.c
FW = $(GAME) ../fw/flush.c

TEST_BINS = tests/test_max7219 tests/test_avr128db28 tests/test_board tests/test_game

retrodiode-ui: main.c hal.c $(CORE) $(FW)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main.c hal.c $(CORE) $(FW) $(LDLIBS)

tests/test_max7219: tests/test_max7219.c chips/max7219.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_max7219.c chips/max7219.c engine/signal.c

tests/test_avr128db28: tests/test_avr128db28.c chips/avr128db28.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_avr128db28.c chips/avr128db28.c engine/signal.c

tests/test_board: tests/test_board.c hal.c $(CORE) $(FW)
	$(CC) $(CFLAGS) -o $@ tests/test_board.c hal.c $(CORE) $(FW)

tests/test_game: tests/test_game.c $(GAME)
	$(CC) $(CFLAGS) -o $@ tests/test_game.c $(GAME)

test: $(TEST_BINS)
	./tests/test_max7219
	./tests/test_avr128db28
	./tests/test_game
	./tests/test_board

clean:
	rm -f retrodiode-ui test_chips $(TEST_BINS)
