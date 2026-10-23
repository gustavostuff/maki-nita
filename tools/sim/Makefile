# From the repo root:
#   make -C tools/sim
#   ./tools/sim/maki-nita-ui
#   make -C tools/sim test

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I../fw -I../../games/space-shooter -I../../games/paddle -I.
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf)
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf)

CORE = engine/signal.c chips/max7219.c chips/avr128db28.c board.c ../fw/wire.c
COMMON = ../fw/frame.c ../fw/input.c ../fw/score.c ../fw/text.c ../fw/over.c
SHOOTER = ../../games/space-shooter/game.c \
          ../../games/space-shooter/entity/base_entity.c \
          ../../games/space-shooter/entity/player.c \
          ../../games/space-shooter/entity/bullet.c \
          ../../games/space-shooter/entity/rock.c
PADDLE = ../../games/paddle/paddle.c
FW = $(COMMON) $(SHOOTER) ../fw/flush.c

TEST_BINS = tests/test_max7219 tests/test_avr128db28 tests/test_board tests/test_game \
            tests/test_paddle tests/test_cart tests/test_synth

maki-nita-ui: main.c hal.c persist.c sound.c cart.c $(CORE) $(FW) $(PADDLE) ../fw/synth.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main.c hal.c persist.c sound.c cart.c $(CORE) $(FW) $(PADDLE) ../fw/synth.c $(LDLIBS)

tests/test_max7219: tests/test_max7219.c chips/max7219.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_max7219.c chips/max7219.c engine/signal.c

tests/test_avr128db28: tests/test_avr128db28.c chips/avr128db28.c engine/signal.c
	$(CC) $(CFLAGS) -o $@ tests/test_avr128db28.c chips/avr128db28.c engine/signal.c

tests/test_board: tests/test_board.c hal.c $(CORE) $(FW) ../fw/persist_ram.c tests/sound_log.c
	$(CC) $(CFLAGS) -o $@ tests/test_board.c hal.c $(CORE) $(FW) ../fw/persist_ram.c tests/sound_log.c

tests/test_game: ../../games/space-shooter/test.c $(COMMON) $(SHOOTER) ../fw/persist_ram.c tests/sound_log.c
	$(CC) $(CFLAGS) -o $@ ../../games/space-shooter/test.c $(COMMON) $(SHOOTER) ../fw/persist_ram.c tests/sound_log.c

tests/test_paddle: ../../games/paddle/test.c $(COMMON) $(PADDLE) ../fw/persist_ram.c
	$(CC) $(CFLAGS) -o $@ ../../games/paddle/test.c $(COMMON) $(PADDLE) ../fw/persist_ram.c

tests/test_cart: tests/test_cart.c cart.c $(COMMON) $(SHOOTER) $(PADDLE) ../fw/persist_ram.c tests/sound_log.c
	$(CC) $(CFLAGS) -o $@ tests/test_cart.c cart.c $(COMMON) $(SHOOTER) $(PADDLE) ../fw/persist_ram.c tests/sound_log.c

tests/test_synth: tests/test_synth.c ../fw/synth.c ../fw/wire.c
	$(CC) $(CFLAGS) -o $@ tests/test_synth.c ../fw/synth.c ../fw/wire.c

test: $(TEST_BINS)
	./tests/test_max7219
	./tests/test_avr128db28
	./tests/test_game
	./tests/test_paddle
	./tests/test_cart
	./tests/test_board
	./tests/test_synth

clean:
	rm -f maki-nita-ui makinita-ui retrodiode-ui test_chips $(TEST_BINS)
