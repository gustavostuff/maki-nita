# AVR128DB28 image. Needs avr-gcc with that device.
#   make -C tools/fw/avr
#   make -C tools/fw/avr CART=paddle
#   pymcuprog write -d avr128db28 -t uart -u /dev/ttyACM0 --verify -f game.hex

MCU = avr128db28
CLOCK = 24000000
CC = avr-gcc
OBJCOPY = avr-objcopy
CART ?= space-shooter
CART_DIR = ../../../games/$(CART)

ifeq ($(CART),paddle)
GAME_SRCS = $(CART_DIR)/paddle.c
else ifeq ($(CART),space-shooter)
GAME_SRCS = $(CART_DIR)/game.c \
            $(CART_DIR)/entity/base_entity.c $(CART_DIR)/entity/player.c \
            $(CART_DIR)/entity/bullet.c $(CART_DIR)/entity/rock.c
else
$(error CART must be space-shooter or paddle)
endif

CFLAGS = -std=c11 -Wall -Wextra -Os -mmcu=$(MCU) -DF_CPU=$(CLOCK)UL -I$(CART_DIR) -I..
LDFLAGS = -mmcu=$(MCU)

SRCS = main.c hal.c sound.c $(GAME_SRCS) ../frame.c ../input.c ../flush.c \
       ../score.c ../text.c ../over.c persist.c

game.elf: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

game.hex: game.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f game.elf game.hex square.elf square.hex
