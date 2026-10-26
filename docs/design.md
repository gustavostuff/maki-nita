# Console design

The electrical source of truth is `tools/kicad/skidl/maki_nita.py`. `tools/kicad/skidl/export_netlist.sh` writes `tools/kicad/skidl/maki-nita.net` and patches net classes in `tools/kicad/main-pcb/maki-nita.kicad_pro`. The schematic and PCB live under `tools/kicad/main-pcb/`. The cartridge netlist is `tools/kicad/cart-pcb/skidl/`.

The console, the cartridge, and the rest of the hardware design are under the CERN Open Hardware Licence Version 2, Strongly Reciprocal.

## Console board

The console is a 2-layer board with ground pours on both sides. Parts are through-hole on a 2.54 mm grid. The game AVR is on the cartridge, not this board.

| Ref | Role |
| --- | --- |
| J1 | EDAC 345 cart socket, 16 contacts |
| J2 | 2x4 header, stick and button |
| J3 | 2x2 header, 5 V and reset |
| J4 | 2x2 header, speaker |
| J5 | 1x3 header, audio AVR UPDI |
| U1 to U4 | MAX7219, one per 8x8 block |
| U5 | AVR128DB28, audio and planned brightness control |
| U6 | LM386N-1 amplifier |
| DS1 to DS4 | 8x8 common-cathode matrices |

A part on Un uses n. A second part on that IC adds 10, a third adds 20. For instance:

- On U5: C5 is the first bypass, C15 the second, C25 the third.
- On U1: R1 is ISET, C1 is the 100 nF bypass, C11 is the 10 uF bulk.

Power enters on J3. The barrel jack, polyfuse, Schottky, and panel switch are off the board (design TBD).

## Display

Four white or neutral-white matrices form a 16x16 field. DS1 is top left, DS2 top right, DS3 bottom left, DS4 bottom right. Each driver sits with its matrix. DIN, CLK, and LOAD come from the cart through J1 into U1. DOUT chains U1 to U2 to U3 to U4.

The cart AVR keeps the frame and shifts it over DIN, CLK, and LOAD at 12 MHz. Each MAX7219 scans its block. Segment pins source anodes. Digit pins sink cathodes.

| Block | Screen columns | Screen rows | Chain order |
| --- | --- | --- | --- |
| DS1 | 0 to 7 | 0 to 7 | First |
| DS2 | 8 to 15 | 0 to 7 | Second |
| DS3 | 0 to 7 | 8 to 15 | Third |
| DS4 | 8 to 15 | 8 to 15 | Fourth |

## Cart edge (J1)

Side A is odd contacts. Side B is even contacts. Pad 1 is A1.

| Contact | Side A | Side B |
| --- | --- | --- |
| 1 | GND | EVENT to U5 |
| 2 | +5 V | +5 V |
| 3 | UP | LOAD |
| 4 | DOWN | DIN |
| 5 | LEFT | CLK |
| 6 | RIGHT | RESET |
| 7 | BUTTON | UPDI (game AVR) |
| 8 | GND | +5 V |

Stick and button wires land on J2 and share the A-side nets with J1. RESET on J3 ties to cart contact B6 and U5 reset.

## Audio (U5, U6, J4)

The cart drives EVENT on contact B1 from PA0. U5 listens on PA1 and plays rows from `tools/fw/synth.c`. The mix goes to the DAC, through the divider, into U6, and out to J4. U5 runs at 20 MHz. The byte is eight low pulses on EVENT, 100 us or 200 us wide, sampled at 40 kHz on the console. The mix updates at 20 kHz.

## Net classes

From `tools/kicad/skidl/net_classes.py`: 5V is 0.8 mm track width. MATRIX is segment and digit nets per driver block. AUDIO is DAC, AUD, AUDIN, AMP, SPK, ZOBEL, and BYPASS. Default is 0.25 mm. GND is the pour.

## Not in the SKiDL netlist yet

Global brightness (Bright button, console AVR writing MAX7219 intensity), marquee backlight LEDs, and extra console wiring for those features. The playfield is the 256 LEDs from the four matrices only.

Product notes from the README (magnetic marquee, colored overlays, cart behind the header) are mechanical and are not fixed in KiCad.
