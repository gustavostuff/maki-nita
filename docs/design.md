# Console design

The electrical source of truth is `tools/kicad/skidl/maki_nita.py`. `tools/kicad/skidl/export_netlist.sh` writes `tools/kicad/skidl/maki-nita.net` and patches net classes in `tools/kicad/main-pcb/maki-nita.kicad_pro`. The schematic and PCB live under `tools/kicad/main-pcb/`. The cartridge netlist is `tools/kicad/cart-pcb/skidl/`.

## Console board

The console is a 100x100 mm 2-layer board with ground pours on both sides. Parts are through-hole on a 2.54 mm grid. The game AVR is on the cartridge PCB. Six M3 (3.2 mm) mounting holes (H1 to H6) secure the corners and top edge.

All external headers and the cart socket (J1 to J6) sit along the top area of the board. The 16x16 display field occupies the central and lower area.

| Ref | Role |
| --- | --- |
| J1 | EDAC 395 cart socket, 20 contacts |
| J2 | 2x3 header, Player 1 controls (four directions, button, GND) |
| J3 | 2x3 header, Player 2 controls (four directions, button, GND) |
| J4 | 2x2 header, 5 V and reset |
| J5 | 2x2 header, speaker |
| J6 | 1x3 header, audio AVR UPDI |
| U1 to U4 | MAX7219, one per 8x8 block |
| U5 | AVR128DB28, audio and planned brightness control |
| U6 | LM386N-1 amplifier |
| DS1 to DS4 | 8x8 common-cathode matrices |

A part on "UN" uses "N" as part of its silkscreen label. A second part on that IC adds 10, a third adds 20. For instance:

- On U5: C5 is the first bypass, C15 the second, C25 the third.
- On U1: R1 is ISET, C1 is the 100 nF bypass, C11 is the 10 uF bulk.

R1 to R4 are horizontal axial resistors on a 10.16 mm pitch. Audio and event resistors stand vertical on a 2.54 mm pitch.

Power enters on J4. The barrel jack, polyfuse, Schottky, and panel switch are off the board (design TBD).

## Display

Four white matrices form a 16x16 field. DS1 is top left, DS2 top right, DS3 bottom left, DS4 bottom right. 

Each matrix sits in two elevated 1x8 female pin socket strips. Its MAX7219 driver (U1 to U4) and 100 nF ceramic bypass capacitor (C1 to C4) sit nested directly underneath the matrix body between the socket rows. Bulk 10 uF electrolytic capacitors (C11 to C14) sit slightly outside the matrix area.

Vertical clearance under the matrix body depends on socket height and needs physical verification during assembly. If a DIP-24 socket plus the MAX7219 package is too tall to clear the seated matrix, the drivers can be soldered directly to the board without IC sockets. Because the MAX7219 is fixed-function silicon without firmware or flashing needs, sockets are primarily valuable for the AVR microcontrollers.

DIN, CLK, and LOAD come from the cart through J1 into U1. DOUT chains U1 to U2 to U3 to U4.

The cart AVR keeps the frame and shifts it over DIN, CLK, and LOAD at 12 MHz. Each MAX7219 scans its block. Segment pins source anodes. Digit pins sink cathodes.

The MAX7219 multiplexes its eight rows using an autonomous internal oscillator at ~800 Hz (~1.25 ms per full 8-row cycle). This scan frequency is fixed in hardware and cannot be reconfigured in software. During each 16.67 ms (60 Hz) game frame, the display completes roughly 13 full refresh cycles. Because driver registers update atomically on the rising edge of LOAD, frame flushes remain clean without tearing or flicker.

| Block | Screen columns | Screen rows | Chain order |
| --- | --- | --- | --- |
| DS1 | 0 to 7 | 0 to 7 | First |
| DS2 | 8 to 15 | 0 to 7 | Second |
| DS3 | 0 to 7 | 8 to 15 | Third |
| DS4 | 8 to 15 | 8 to 15 | Fourth |

## Cart edge (J1)

The cart socket is an EDAC 395-020-520-202 with 20 contacts (10 contacts per side, 2.54 mm pitch).

Side A pins sit at the bottom part of the connector on the main PCB, carrying player inputs. Side B pins sit at the top part of the connector on the main PCB, carrying power, audio events, reset, programming, and display signals.

| Contact | Side A | Side B |
| --- | --- | --- |
| 1 | P1_UP | CLK |
| 2 | P1_DOWN | LOAD |
| 3 | P1_LEFT | DIN |
| 4 | P1_RIGHT | GND |
| 5 | P1_BTN | GND |
| 6 | P2_UP | UPDI |
| 7 | P2_DOWN | RESET |
| 8 | P2_LEFT | EVENT |
| 9 | P2_RIGHT | GND |
| 10 | P2_BTN | +5V |

Player 1 cabinet stick and button wires land on J2 (a 2x3 header with pins 5, 3, 1 carrying RIGHT, UP, DOWN and pins 6, 4, 2 carrying LEFT, GND, BUTTON) and route to cart contacts A1 through A5. Player 2 stick and button wires land on J3 from the side connector (also a 2x3 header with the same pin arrangement) and route to cart contacts A6 through A10. RESET on J4 ties to cart contact B7 and U5 reset.

## Audio (U5, U6, J5)

U5 sits along the left edge of the board. U6 and the speaker header J5 sit along the right edge.

The cart drives EVENT on contact B8 from PA0. U5 listens on PA1 and plays rows from `tools/fw/synth.c`. The mix goes to the DAC, through the divider, into U6, and out to J5. U5 runs at 20 MHz. The byte is eight low pulses on EVENT, 100 us or 200 us wide, sampled at 40 kHz on the console. The mix updates at 20 kHz.

## Net classes

From `tools/kicad/skidl/net_classes.py`: 5V is 0.8 mm track width. MATRIX is segment and digit nets per driver block, routed at 0.4 mm width instead of the default 0.25 mm. Each digit pin sinks up to eight LEDs simultaneously, carrying peak return currents around 160 mA. The 0.4 mm width lowers line resistance and voltage drop across the scan cycle, keeping LED brightness uniform between digits and reducing switching noise into adjacent lines. AUDIO is DAC, AUD, AUDIN, AMP, SPK, ZOBEL, and BYPASS. Default is 0.25 mm. GND is the pour.

## Not in the SKiDL netlist yet

Global brightness (Bright button with tap-to-increment and hold-plus-stick adjustment, console AVR writing MAX7219 intensity) and extra console wiring for that feature. The playfield is the 256 LEDs from the four matrices only.
