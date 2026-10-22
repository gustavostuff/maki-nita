# Maki Nita hardware

Maki Nita is one board. Sixteen red 8x8 matrices fill the top in a 4 by 4 grid. A strip along the bottom holds the sixteen display drivers, the CPU, the power parts, and the headers. The field is about 128 mm square. The strip makes the board a little wider and taller than that square.

The stick and the button mount to the case. A header on the bottom edge of the strip is where their wires land.

Each matrix is a 16-pin common-cathode part, about 32 mm square. The packages touch. Rows are the cathodes. Columns are the anodes. Every matrix sits in the same orientation, so the same package corner points the same way on every block. The builder solders the matrices. The drivers, the CPU, and the passives are parts on the board.

```mermaid
flowchart LR
  controls[Stick and button] --> avr[AVR128DB28]
  avr -->|data, clock, load| drivers[16x MAX7219]
  drivers -->|segment and digit lines| leds[32x32 red dots]
```

## Who turns a pixel on

The AVR reads the stick and the button, keeps the frame in RAM, and sends that frame to the drivers. It does not drive an LED pin.

The five inputs are active low, with the pin's own pull-up. A closed switch ties the pin to GND. Up, down, left, right, and the button are PD1 through PD5, package pins 7 through 11.

The three outputs are the display chain. DIN is PA4, package pin 26, and it idles low. CLK is PA6, package pin 28, and it idles low. A rising edge of CLK shifts one bit into every MAX7219. LOAD is PA7, package pin 1, and it idles high. A rising edge of LOAD latches the 16-bit word each driver is holding.

Each MAX7219 stores one 8x8 block and scans it on its own. A segment pin sources an LED anode. A digit pin sinks an LED cathode. A lit pixel is one segment bit set in that digit's register.

The blocks form a 4 by 4 array. Block column `bc` and block row `br` start at 0 in the top left. That block covers screen columns `bc * 8` through `bc * 8 + 7` and screen rows `br * 8` through `br * 8 + 7`. The chain runs left to right across a row, then down to the next row. The top-left block is first and takes DIN from the AVR. The bottom-right block is last.

Column 0 is the left of the screen. Row 0 is the top. Inside a block, the same rule holds. The block's own column 0 is its left edge, and its own row 0 is its top edge.

DOUT of one driver feeds DIN of the next. CLK is common. LOAD is common. DOUT changes on the falling edge of CLK. A bit at DIN is valid at DOUT 16.5 clock cycles later. In a sixteen-driver transfer, the first 16-bit word is the word the last driver latches.

## Parts

As of October 2026, Microchip lists the AVR128DB28-I/SP as in production, and Analog Devices lists the MAX7219ENG+ as in production. Neither part is end of life, and neither is discouraged for a new design.

| Qty | Part | Role |
| --- | --- | --- |
| 1 | AVR128DB28-I/SP | CPU. 128 KB flash, 16 KB RAM, 28-pin SPDIP |
| 1 | 28-pin DIP socket | For the CPU |
| 16 | MAX7219ENG+ | 8x8 common-cathode LED drivers, 24-pin DIP |
| 16 | 24-pin DIP sockets, 0.3 inch | For the drivers |
| 16 | Red 8x8 matrix, common cathode, about 32 mm square | The field. 16 pins |
| 16 | 28 kohm | ISET, one per driver. About 20 mA peak per LED |
| 1 | 10 kohm | RESET pull-up |
| 1 | 10 kohm | LOAD pull-up |
| 2 | 10 kohm | Pull-downs on DIN and CLK |
| 1 | SB540 | Series Schottky, 5 A, reverse-plug protection |
| 1 | Polyfuse, 4 A hold | On the 5 V input |
| 1 | Barrel jack, 5.5 x 2.1 mm, center positive | Regulated 5 V in |
| 1 | SPST panel switch | Breaks the 5 V rail |
| 1 | 470 uF electrolytic | Bulk capacitor on the 5 V rail |
| 16 | 10 uF electrolytic | One from each driver V+ to GND, close to the package |
| 19 | 100 nF | One at each driver V+ pin, and at VDD, AVDD, and VDDIO2 |

VDD, AVDD, and VDDIO2 all tie to the 5 V rail. Port C takes power from VDDIO2. An open VDDIO2 pin leaves that port unpowered.

Each driver has two GND pins, 4 and 9. Both tie to ground. The 10 uF and the 100 nF for that driver sit between its V+ and GND, close to the package.

DIN, CLK, and LOAD are logic inputs with a high threshold of 3.5 V and a low threshold of 0.8 V. The AVR drives them from the same rail as the drivers.

Each ISET resistor sits between that driver's V+ and its ISET pin. For a red LED near 2 V, 28 kohm sets the peak segment current near 20 mA. The drivers also have a global intensity register. It stays at full scale (31/32). Games do not change it. A lit pixel then averages about 2.4 mA, because the driver scans one row of its block at a time.

## Block wiring

Segment pins are anodes, left to right. The digit byte matches that order. Bit 7 is the left pixel of the block.

| Column in the block | Segment pin | Bit |
| --- | --- | --- |
| 0 | DP | 7 |
| 1 | A | 6 |
| 2 | B | 5 |
| 3 | C | 4 |
| 4 | D | 3 |
| 5 | E | 2 |
| 6 | F | 1 |
| 7 | G | 0 |

Digit pins are cathodes, top to bottom. Digit 0 is the top row of the block.

| Row in the block | Digit pin | Register address |
| --- | --- | --- |
| 0 | DIG0 | 0x01 |
| 1 | DIG1 | 0x02 |
| 2 | DIG2 | 0x03 |
| 3 | DIG3 | 0x04 |
| 4 | DIG4 | 0x05 |
| 5 | DIG5 | 0x06 |
| 6 | DIG6 | 0x07 |
| 7 | DIG7 | 0x08 |

The matrix datasheet names which package pin is which row and which column. Those pins land on the segment and digit lines above. A segment that is off is pulled to GND. A digit that is off is pulled to V+.

On power-up the MAX7219 blanks the display, enters shutdown, sets the scan limit to digit 0 only, leaves decode off, and sets intensity to its minimum. Firmware writes scan limit, decode mode, and intensity before it leaves shutdown. The driver can take those writes while shutdown is still in force.

## Current

A full-white frame turns on every LED. Each driver lights one of its eight rows at a time, eight LEDs at about 20 mA, and blanks that row for 1/32 of the row time. One driver therefore draws about 155 mA for the LEDs. Sixteen drivers draw about 2.5 A. The dissipation formula in the driver datasheet uses 8 mA of logic current per driver, and the CPU draws under 20 mA, so the rest of the board is about 150 mA. The total is about 2.6 A.

The jack takes a regulated 5 V supply, 5 A. Center positive. The polyfuse, then the Schottky, then the panel switch, then the bulk capacitor. The polyfuse holds 4 A, so a full-white frame leaves it closed. The Schottky drops about half a volt at this current. The drivers are specified from 4.0 V to 5.5 V, so the rail stays in range.

At 20 mA and a red LED near 2 V, each DIP dissipates well under 1 W. The packages stay within their thermal rating on a full-white frame.

## CPU signals

Port pins on the AVR128DB28. Package pin numbers are the 28-pin SPDIP numbers from the datasheet.

| Signal | Package pin | Port | Direction | Notes |
| --- | --- | --- | --- | --- |
| DIN | 26 | PA4 | out | MOSI into the first MAX7219. 10 kohm pull-down. Idle low |
| CLK | 28 | PA6 | out | Rising edge clocks one bit. 10 kohm pull-down. Idle low |
| LOAD | 1 | PA7 | out | Rising edge latches the 16 bits in each driver's own shift register. 10 kohm pull-up. Idle high |
| UP | 7 | PD1 | in | Active low, internal pull-up |
| DOWN | 8 | PD2 | in | Active low, internal pull-up |
| LEFT | 9 | PD3 | in | Active low, internal pull-up |
| RIGHT | 10 | PD4 | in | Active low, internal pull-up |
| BUTTON | 11 | PD5 | in | Active low, internal pull-up |
| RESET | 18 | PF6 | reset | 10 kohm pull-up. Stays a reset pin |
| UPDI | 19 | UPDI | program | Programming header |

PA0, PA1, PA2, PA3, PA5, PC0, PC1, PC2, PC3, PD6, PD7, PF0, and PF1 stay free. PF0 and PF1 are the 32 kHz crystal pins, so they carry no loads.

The programming header is a 1x3: UPDI, VCC, GND. The board is powered from the barrel jack during programming.

## Controls

The stick is a Sanwa JLF or the same kind of part. Four independent switches. A diagonal is two switches closed at once. A square gate or an octagonal gate is a mechanical plate. The circuit is the same either way.

Every switch common ties to GND. The normally open contact ties to its signal pin. The button is the same kind of switch. One 6-pin header on the bottom edge: GND, UP, DOWN, LEFT, RIGHT, BUTTON. Those five signals are PD1 through PD5.

## Still open

- A case.
- A small speaker for beeps. It plays a triangle or a sine wave at a chosen frequency. The pin and the drive circuit are still open. The other spare pins stay free.
