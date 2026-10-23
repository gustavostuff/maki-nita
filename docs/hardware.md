# Maki Nita hardware

Maki Nita is a console and a cartridge. Four red 8x8 matrices form a 2 by 2 field, about 64 mm square. The console also carries the four display drivers, the audio AVR, the amplifier, the cart socket, the power parts, and the headers. The cartridge carries the AVR128DB28 and nothing else. That chip is the CPU.

The stick, the button, and the speaker mount to the case. Their wires land on headers. Power and reset land on another.

The console, the cartridge, and the rest of the hardware design are under the CERN Open Hardware Licence Version 2, Strongly Reciprocal.

Each matrix is a 16-pin common-cathode part, about 32 mm square. The packages touch. Rows are the cathodes. Columns are the anodes. Every matrix sits in the same orientation, so the same package corner points the same way on every block. The builder solders the matrices. The drivers, the audio parts, and the passives are parts on the console. The CPU is a part on the cartridge.

```mermaid
flowchart LR
  controls[Stick and button] --> socket[Cart socket]
  cart[AVR128DB28 cartridge] --> socket
  socket -->|data, clock, load| drivers[4x MAX7219]
  drivers -->|segment and digit lines| leds[16x16 red dots]
  socket -->|EVENT| audio[Audio AVR128DB28]
  audio -->|triangle mix| amp[LM386]
  amp --> speaker[8 ohm speaker]
```

## Who turns a pixel on

The AVR reads the stick and the button, keeps the frame in RAM, and sends that frame to the drivers. It does not drive an LED pin.

The five inputs are active low, with the pin's own pull-up. A closed switch ties the pin to GND. Up, down, left, right, and the button are PD1 through PD5, package pins 7 through 11.

The three outputs are the display chain. DIN is PA4, package pin 26, and it idles low. CLK is PA6, package pin 28, and it idles low. A rising edge of CLK shifts one bit into every MAX7219. LOAD is PA7, package pin 1, and it idles high. A rising edge of LOAD latches the 16-bit word each driver is holding.

Each MAX7219 stores one 8x8 block and scans it on its own. A segment pin sources an LED anode. A digit pin sinks an LED cathode. A lit pixel is one segment bit set in that digit's register.

| Block | Columns | Rows | Place in the chain |
| --- | --- | --- | --- |
| Top left | 0 to 7 | 0 to 7 | First. DIN comes from the AVR |
| Top right | 8 to 15 | 0 to 7 | Second |
| Bottom left | 0 to 7 | 8 to 15 | Third |
| Bottom right | 8 to 15 | 8 to 15 | Fourth. End of the chain |

Column 0 is the left of the screen. Row 0 is the top. Inside a block, the same rule holds. The block's own column 0 is its left edge, and its own row 0 is its top edge.

DOUT of one driver feeds DIN of the next. CLK is common. LOAD is common. DOUT changes on the falling edge of CLK. A bit at DIN is valid at DOUT 16.5 clock cycles later. In a four-driver transfer, the first 16-bit word is the word the last driver latches.

## Parts

As of October 2026, Microchip lists the AVR128DB28-I/SP as in production, and Analog Devices lists the MAX7219ENG+ and the MAX7219CNG+ as in production. None of these parts is end of life, and none is discouraged for a new design.

| Qty | Part | Role |
| --- | --- | --- |
| 1 | EDAC 345-016-520-212 | Cart socket on the console. 16 contacts, 8 a side, 2.54 mm pitch |
| 1 | Pin header, 2x4, 2.54 mm | Stick and button. J2 |
| 1 | Pin header, 2x2, 2.54 mm | Power and reset. J3 |
| 1 | Pin header, 1x2, 2.54 mm | Speaker. J4 |
| 1 | Pin header, 1x3, 2.54 mm | Audio programming. J5 |
| 1 | AVR128DB28-I/SP | CPU, on the cartridge. 128 KB flash, 16 KB RAM, 512 bytes EEPROM, 28-pin SPDIP |
| 1 | 28-pin DIP socket | For the CPU, on the cartridge |
| 1 | AVR128DB28-I/SP | Audio, on the console. Same part as the CPU |
| 1 | 28-pin DIP socket | For the audio AVR |
| 1 | LM386N-1 | Speaker amplifier, DIP-8. Gain stays 20 |
| 1 | 8-pin DIP socket, 0.3 inch | For the amplifier |
| 4 | MAX7219ENG+ or MAX7219CNG+ | 8x8 common-cathode LED drivers, 24-pin narrow DIP |
| 4 | 24-pin DIP sockets, 0.3 inch | For the drivers |
| 4 | Red 8x8 matrix, common cathode, about 32 mm square | The field. 16 pins |
| 4 | 28 kohm | ISET, one per driver. About 20 mA peak per LED |
| 1 | 10 kohm | RESET pull-up |
| 1 | 10 kohm | LOAD pull-up |
| 1 | 10 kohm | EVENT pull-up |
| 1 | 10 kohm | DAC load, from the audio AVR PD6 to GND |
| 1 | 22 kohm | Series resistor into the LM386 |
| 1 | 1 kohm | Shunt at the LM386 input |
| 1 | 10 ohm | Zobel resistor at the LM386 output |
| 2 | 10 kohm | Pull-downs on DIN and CLK |
| 1 | 1N5822 | Series Schottky, reverse-plug protection |
| 1 | Polyfuse, 1.5 A hold | On the 5 V input |
| 1 | Barrel jack, 5.5 x 2.1 mm, center positive | Regulated 5 V in |
| 1 | SPST panel switch | Breaks the 5 V rail |
| 1 | 100 uF electrolytic | Bulk capacitor on the 5 V rail |
| 1 | 100 uF electrolytic | LM386 supply, close to the package |
| 1 | 220 uF electrolytic | LM386 output coupling to the speaker |
| 1 | 1 uF electrolytic | DAC coupling into the divider. Positive faces the DAC |
| 4 | 10 uF electrolytic | One from each driver V+ to GND, close to the package |
| 1 | 47 nF | Zobel capacitor, in series with the 10 ohm |
| 11 | 100 nF | One at each driver V+ pin, at VDD, AVDD, and VDDIO2 of each AVR, and at the LM386 supply |

The MAX7219CNG+ is the commercial grade of the same narrow plastic DIP as the MAX7219ENG+. Datasheet revision 6 gives both parts one pin diagram. DIN is pin 1, LOAD is pin 12, CLK is pin 13, ISET is pin 18, V+ is pin 19, and DOUT is pin 24. The digit and segment pins match the block wiring below. The electrical limits are the same table, checked across each part's own temperature range. Analog Devices rates the CNG+ from 0 to 70 degrees C, and the ENG+ from -40 to 85 degrees C.

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

A full-white frame turns on every LED. Each driver lights one of its eight rows at a time, eight LEDs at about 20 mA, and blanks that row for 1/32 of the row time. One driver therefore draws about 155 mA for the LEDs. Four drivers draw about 620 mA. The dissipation formula in the driver datasheet uses 8 mA of logic current per driver, about 32 mA for all four. Each AVR draws under 20 mA. The LM386 draws a few milliamps at idle. Besides the LEDs, that is about 80 mA, and the total is about 0.7 A. A loud tone into an 8 ohm speaker can add up to about 200 mA. The 1.5 A fuse stays closed on a full-white frame plus that tone, and the 2 A adapter still covers it.

The jack takes a regulated 5 V supply, 2 A. Center positive. The polyfuse, then the Schottky, then the panel switch, then the bulk capacitor. The polyfuse holds 1.5 A, so a full-white frame plus a loud tone leaves it closed. The Schottky drops about 0.4 V. The drivers are specified from 4.0 V to 5.5 V, so the rail near 4.6 V is in range. That rail enters the console on J3.

At 20 mA and a red LED near 2 V, each DIP dissipates well under 1 W. The packages stay within their thermal rating on a full-white frame.

## Layers

The console and the cartridge are 2-layer boards. Both sides carry a ground pour, and vias stitch the pours together. The segment and digit traces of one matrix stay close to each other. DIN, CLK, and LOAD run over the pour.

The frame flushes once per 60 Hz tick. One flush is 512 rising edges of CLK, about 31,000 edges a second. The burst lasts about a millisecond at a few hundred kilohertz. CLK and DIN then sit low, and LOAD sits high.

Each driver scans its block at about 800 Hz. A full row is about 160 mA peak. The 10 uF and the 100 nF at each driver V+ pin take that pulse. The stick wires leave beside the ground pins on J2. The reset switch leaves beside the ground pin on J3.

## Cartridge

The console socket is an EDAC 345-016-520-212. It is a straight card-edge connector from the [345/395 series](https://files.edac.net/edac/content/series/og/English/EDAC%20345%20395%20Series%20Card%20Edge%20Connectors%20English%20Ordering%20Guide.pdf), 2.54 mm along a row and 5.08 mm between the two rows. Sixteen contacts, eight a side. Side A is the odd pad numbers and side B is the even pad numbers. Pad 1 is contact A1.

The cartridge is a separate board. It holds the AVR128DB28 and a double-sided gold edge, eight fingers a side at 2.54 mm. Side A is the front copper and carries the stick and the button. Side B is the back copper and carries the display, EVENT, and the programming pins. VDD, AVDD, and VDDIO2 tie to the +5 V fingers. Both ground pins tie to the GND fingers. Two contacts are GND and three are +5 V. Ground sits at both ends of the edge, on A1 and A8.

| Contact | Side A | Side B |
| --- | --- | --- |
| 1 | GND | EVENT, AVR pin 22 |
| 2 | +5 V | +5 V |
| 3 | UP, AVR pin 7 | LOAD, AVR pin 1 |
| 4 | DOWN, AVR pin 8 | DIN, AVR pin 26 |
| 5 | LEFT, AVR pin 9 | CLK, AVR pin 28 |
| 6 | RIGHT, AVR pin 10 | RESET, AVR pin 18 |
| 7 | BUTTON, AVR pin 11 | UPDI, AVR pin 19 |
| 8 | GND | +5 V |

## CPU signals

Port pins on the AVR128DB28. Package pin numbers are the 28-pin SPDIP numbers from the datasheet. The contact column is the cartridge edge.

| Signal | Package pin | Contact | Port | Direction | Notes |
| --- | --- | --- | --- | --- | --- |
| DIN | 26 | B4 | PA4 | out | MOSI into the first MAX7219. 10 kohm pull-down. Idle low |
| CLK | 28 | B5 | PA6 | out | Rising edge clocks one bit. 10 kohm pull-down. Idle low |
| LOAD | 1 | B3 | PA7 | out | Rising edge latches the 16 bits in each driver's own shift register. 10 kohm pull-up. Idle high |
| EVENT | 22 | B1 | PA0 | out | Index byte to the audio AVR. 10 kohm pull-up on the console. Idle high |
| UP | 7 | A3 | PD1 | in | Active low, internal pull-up |
| DOWN | 8 | A4 | PD2 | in | Active low, internal pull-up |
| LEFT | 9 | A5 | PD3 | in | Active low, internal pull-up |
| RIGHT | 10 | A6 | PD4 | in | Active low, internal pull-up |
| BUTTON | 11 | A7 | PD5 | in | Active low, internal pull-up |
| RESET | 18 | B6 | PF6 | reset | 10 kohm pull-up. Stays a reset pin |
| UPDI | 19 | B7 | UPDI | program | Programming header |

PA1, PA2, PA3, PA5, PC0, PC1, PC2, PC3, PD6, PD7, PF0, and PF1 stay free. PF0 and PF1 are the 32 kHz crystal pins, so they carry no loads.

The game chip's programming header is a 1x3: UPDI, VCC, GND. The board is powered from the barrel jack during programming.

## Controls

The stick is a Sanwa JLF or the same kind of part. Four independent switches. A diagonal is two switches closed at once. A square gate or an octagonal gate is a mechanical plate. The circuit is the same either way.

Every switch common ties to GND. The normally open contact ties to its signal pin. The button is the same kind of switch. Those five signals are PD1 through PD5. They leave the board on J2, a 2x4 pin header, 2.54 mm pitch, and reach side A of the cart socket. Pin 1 is the square pad.

| Pin | Net |
| --- | --- |
| 1 | GND |
| 2 | +5 V |
| 3 | UP |
| 4 | DOWN |
| 5 | LEFT |
| 6 | RIGHT |
| 7 | BUTTON |
| 8 | GND |

## Power and reset

J3 is a 2x2 pin header, 2.54 mm pitch. Pin 1 is the square pad. The row with that pad is the 5 V rail. The other row is reset. RESET is the same net as cart contact B6 and the audio AVR reset pin. A switch from pin 3 to pin 4 pulls both chips into reset. The 10 kohm pull-up holds the net high.

| Pin | Net |
| --- | --- |
| 1 | GND |
| 2 | +5 V |
| 3 | RESET |
| 4 | GND |

## Sound

The console carries a second AVR128DB28 and an LM386N-1. The audio AVR mixes three triangle voices into one sample and writes that sample to its DAC. The LM386 drives an 8 ohm speaker. The speaker wires land on J4. The software doc defines the catalog and the bytes on EVENT.

The audio AVR clocks from the internal 24 MHz oscillator. PF0 and PF1 carry no loads. VDD, AVDD, and VDDIO2 tie to the 5 V rail, each with a 100 nF capacitor. RESET joins the panel switch and cart contact B6, so that switch resets both chips. J5 programs this chip. Cart contact B7 stays the game chip's UPDI.

EVENT is cart contact B1. The game AVR drives it from PA0, push-pull, and idles high. The 10 kohm pull-up holds the net high with the cartridge removed. The audio AVR listens on PA1.

The DAC pin is PD6. The 10 kohm to GND is its DC load. A 1 uF capacitor, positive toward the DAC, couples that pin to a divider. The 22 kohm is in series. The 1 kohm ties the LM386 input to GND. A full mix of about 3.5 V peak to peak at the DAC arrives as about 0.15 V at that input. LM386 pins 1 and 8 stay open, so the gain stays 20. Pin 2 ties to GND. Pin 7 stays open.

The output pin feeds the 220 uF capacitor, positive toward the amplifier, and then J4 pin 1. J4 pin 2 is GND. The 10 ohm and the 47 nF sit in series from the output pin to GND. The 100 uF and a 100 nF sit from the LM386 supply pin to GND, close to the package.

| Signal | Package pin | Port | Direction | Notes |
| --- | --- | --- | --- | --- |
| EVENT | 23 | PA1 | in | From cart contact B1. Idle high |
| DAC | 12 | PD6 | out | DAC0. 10 kohm to GND |
| RESET | 18 | PF6 | reset | Same net as J3 and cart contact B6 |
| UPDI | 19 | UPDI | program | J5 |

J4 is a 1x2 pin header, 2.54 mm pitch. Pin 1 is the square pad.

| Pin | Net |
| --- | --- |
| 1 | Speaker, after the 220 uF |
| 2 | GND |

J5 is a 1x3 pin header, 2.54 mm pitch. Pin 1 is the square pad. The board is powered from the barrel jack during programming.

| Pin | Net |
| --- | --- |
| 1 | UPDI |
| 2 | +5 V |
| 3 | GND |

One index byte on EVENT lasts about 2 ms. The line then sits high.

## Still open

- A case.
