# AVR128DB28 image. Needs avr-gcc with that device.
#   make -C fw/avr
#   pymcuprog write -d avr128db28 -t uart -u /dev/ttyACM0 --verify -f square.hex

MCU = avr128db28
CLOCK = 24000000
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c11 -Wall -Wextra -Os -mmcu=$(MCU) -DF_CPU=$(CLOCK)UL -I..
LDFLAGS = -mmcu=$(MCU)

SRCS = main.c hal.c ../frame.c ../input.c ../game.c ../flush.c

square.elf: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

square.hex: square.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f square.elf square.hex
