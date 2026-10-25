# Catalog image for the console AVR128DB28.
#   make -C tools/fw/audio
#   pymcuprog write -d avr128db28 -t uart -u /dev/ttyACM0 --verify -f audio.hex

MCU = avr128db28
CLOCK = 20000000
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c11 -Wall -Wextra -Os -mmcu=$(MCU) -DF_CPU=$(CLOCK)UL -I..
LDFLAGS = -mmcu=$(MCU)

SRCS = main.c ../synth.c ../wire.c

audio.elf: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

audio.hex: audio.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -f audio.elf audio.hex
