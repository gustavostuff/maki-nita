# From the repo root:
#   make -C tools/sim
#   ./tools/sim/maki-nita-ui
#   make -C tools/sim test

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I../fw -I.
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf)
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf)

CORE = engine/signal.c chips/max7219.c chips/avr128db28.c board.c
GAME = ../fw/frame.c ../fw/input.c ../fw/game.c \
       ../fw/entity/base_entity.c ../fw/entity/player.c \
       ../fw/entity/bullet.c ../fw/entity/rock.c ../fw/score.c ../fw/text.c
FW = $(GAME) ../fw/flush.c

TEST_BINS = tests/test_max7219 tests/test_avr128db28 tests/test_board tests/test_game \
            tests/test_synth

maki-nita-ui: main.c hal.c persist.c sound.c $(CORE) $(FW) ../fw/synth.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main.c hal.c persist.c sound.c $(CORE) $(FW) ../fw/synth.c $(LDLIBS)

tests/test_max7219: tests/test_max7219.c chips/max7219.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_max7219.c chips/max7219.c engine/signal.c

tests/test_avr128db28: tests/test_avr128db28.c chips/avr128db28.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_avr128db28.c chips/avr128db28.c engine/signal.c

tests/test_board: tests/test_board.c hal.c $(CORE) $(FW) ../fw/persist_ram.c tests/sound_log.c
	$(CC) $(CFLAGS) -o $@ tests/test_board.c hal.c $(CORE) $(FW) ../fw/persist_ram.c tests/sound_log.c

tests/test_game: tests/test_game.c $(GAME) ../fw/persist_ram.c tests/sound_log.c
	$(CC) $(CFLAGS) -o $@ tests/test_game.c $(GAME) ../fw/persist_ram.c tests/sound_log.c

tests/test_synth: tests/test_synth.c ../fw/synth.c ../fw/wire.c
	$(CC) $(CFLAGS) -o $@ tests/test_synth.c ../fw/synth.c ../fw/wire.c

test: $(TEST_BINS)
	./tests/test_max7219
	./tests/test_avr128db28
	./tests/test_game
	./tests/test_board
	./tests/test_synth

clean:
	rm -f maki-nita-ui makinita-ui retrodiode-ui test_chips $(TEST_BINS)
