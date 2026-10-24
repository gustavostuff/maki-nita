# Maki-Nita audio

The console carries a second AVR128DB28 and an LM386N-1. The audio AVR holds a catalog of 256 sounds. It mixes a square wave, a triangle, and noise into one sample and writes that sample to its DAC. The LM386 drives an 8 ohm speaker. The speaker wires land on J4.

The notes stay on the console. The cartridge sends one index byte and nothing else. A game may send music, effects, both, or neither. A game that sends nothing leaves the speaker quiet.

The audio AVR clocks from the internal 24 MHz oscillator. PF0 and PF1 carry no loads. VDD, AVDD, and VDDIO2 tie to the 5 V rail, each with a 100 nF capacitor. RESET joins the panel switch and cart contact B6, so that switch resets both chips. J5 programs this chip. Cart contact B7 stays the game chip's UPDI.

```mermaid
flowchart LR
  game[Game AVR PA0] -->|EVENT| audio[Audio AVR PA1]
  audio -->|DAC sample| amp[LM386]
  amp --> speaker[8 ohm speaker]
```

## EVENT

EVENT is cart contact B1. The game AVR drives it from PA0, package pin 22, push-pull, and idles high. The 10 kohm pull-up on the console holds the net high with the cartridge removed. The audio AVR listens on PA1, package pin 23.

The byte is the catalog index, from 0 through 255. It goes out least significant bit first, as eight low pulses. A 0 holds the line low for 100 us. A 1 holds it low for 200 us. The line returns high for 100 us between pulses. The audio AVR samples that line at 40 kHz and times each pulse itself. The two chips share no baud rate. One byte lasts about 2 ms, and the line then sits high.

The sender keeps a short queue, so the 60 Hz game tick does not wait on the pulses. The mix is written to the DAC at 20 kHz.

## Catalog

The three channels have fixed waveforms. One is a square wave, one is a triangle, and one is noise. A filled index is one row in `tools/fw/synth.c`. The console image and the simulator both play that file. An index with no row is ignored. Index 0 is empty.

A row names one channel, whether the phrase ends or holds, and a list of steps. A step is a period, a level, and a duration. The period and the duration count samples at 20 kHz. For the noise channel the period is how often a new random bit is drawn. The level runs from 0 to 255.

A later index on the same channel replaces the phrase there. The other channels keep going. A phrase that ends leaves its channel quiet. A phrase that holds stays until another row takes that channel. Level 0 cuts the channel.

The three channels add into one mix, centered on DAC midscale. A quiet channel adds nothing. Three channels at level 255 together are the full mix, about 2.5 V peak to peak at the DAC pin. The DAC reference is the internal 2.5 V source.

Three indexes are filled. Index 4 through 255 are empty. The next sound takes index 4.

| Index | Name | Channel | Phrase | Samples |
| --- | --- | --- | --- | --- |
| 1 | Laser | triangle | ends | 3500 |
| 2 | Explosion | noise | ends | 9000 |
| 3 | Pop | square | ends | 400 |

Index 1 is the laser. Five triangle steps. The first period is 12 samples and the first level is 220. Later steps lengthen the period and lower the level. The phrase ends after 3500 samples. The shooter sends it when a bullet is spawned.

Index 2 is the explosion. Four noise steps. The first period is 1 sample and the first level is 240. Later steps lengthen the period and lower the level. The phrase ends after 9000 samples. The shooter sends it when a bullet removes a rock.

Index 3 is the pop. Two square steps. The first period is 20 samples and the first level is 210. The second step lengthens the period and lowers the level. The phrase ends after 400 samples. Paddle sends it for a wall or the paddle. Arkaled sends it for a wall, a brick, or the paddle. A miss sends nothing.

The software doc is the call a game makes.

## Path

The LM386 is the speaker amplifier. The DAC pin cannot drive an 8 ohm speaker. The amplifier takes the DAC sample and supplies the speaker current.

The DAC pin is PD6. The 10 kohm to GND is its DC load. A 1 uF capacitor, positive toward the DAC, couples that pin to a divider. The 22 kohm is in series. The 1 kohm ties the LM386 input to GND. The full mix arrives as about 0.11 V at that input. LM386 pins 1 and 8 stay open, so the gain stays 20. Pin 2 ties to GND. Pin 7 is the bypass pin. C9, 10 uF, sits from that pin to pin 4.

The output pin feeds the 220 uF capacitor, positive toward the amplifier, and then J4 pins 2 and 3. J4 pins 1 and 4 are GND. The 10 ohm and the 47 nF sit in series from the output pin to GND. C5, 100 uF, and C8, 100 nF, sit from LM386 pin 6 to pin 4, close to the package.

C4, 100 nF, sits across audio AVR pins 20 and 21. C6, 100 nF, sits across pins 14 and 15. C7, 100 nF, sits from pin 6 to pin 21. A net tie joins each of those capacitor nets to the 5 V rail or to ground. The tie is copper on the board. No part is soldered on it.

## Rail noise

The display drivers share the 5 V rail and the ground pour with the audio parts. Each driver scans its block at about 800 Hz. A full row is about 160 mA peak. Those pulses move the rail and the ground.

The DAC reference is the internal 2.5 V source, so the sample does not follow the rail. C9 is the 10 uF the LM386 datasheet uses for supply rejection at the gain of 20. The path that remains is a ground shift between the audio AVR and the LM386 when LED current crosses the pour.

A loud tone into the 8 ohm speaker can add up to about 200 mA. The video doc counts that draw in the console budget. The 1.5 A fuse stays closed, and the 2 A adapter still covers a full-white frame plus that tone.

## Layout

The console is a 2-layer board. Both sides carry a ground pour. A 4-layer board with inner ground planes does not take the scan out of the sample. The firmware reference and C9 do that. A 4-layer stack is a later step, and only if a 2-layer prototype still plays the field in a quiet sound.

U6 and U7 sit next to each other. The DAC pin, the divider, and the LM386 input form one short chain. LED return current does not cross the ground between those two packages. J4 sits next to U7 so the speaker return lands on pin 4.

Vias stitch the two pours around U6, U7, and the four drivers. A via fence around the audio pair is enough. The pour stays one ground. A split analog island that meets digital ground far away puts the LED pulse in series with the sample.

Each net tie sits next to the pin it serves. NT1 through NT3 and NT5 and NT6 sit on the audio AVR. NT4 and NT7 sit on the LM386. The tie is copper. A missing tie leaves that supply pin open.

C4 sits across U6 pins 20 and 21. C6 sits across pins 14 and 15. C7 sits from pin 6 to pin 21. C5, C8, and C9 sit on the LM386, pins 6, 7, and 4. AVDD is tied to VDD inside the chip, so C6 cannot isolate the analog rail.

C1 is electrolytic. The positive pad faces the DAC. C2 is electrolytic. The positive pad faces the LM386 output. C9 is electrolytic. The positive pad faces pin 7.

AUD and AUDIN stay off DIN, CLK, and LOAD. A ferrite in the 5 V feed does not stop an 800 Hz tone. The 4.096 V internal DAC reference sits too close to the rail after the Schottky, about 4.6 V. The 2.5 V source is the one in the image.

The check is a 2-layer board with a full-white field and a quiet sound, or silence. The field in the speaker is the remaining ground shift. A loud phrase covers it.

## Parts

| Qty | Part | Role |
| --- | --- | --- |
| 1 | Pin header, 2x2, 2.54 mm | Speaker. J4 |
| 1 | Pin header, 1x3, 2.54 mm | Audio programming. J5 |
| 1 | AVR128DB28-I/SP | Audio, on the console. Same part as the CPU |
| 1 | 28-pin DIP socket | For the audio AVR |
| 1 | LM386N-1 | Speaker amplifier, DIP-8. Gain stays 20 |
| 1 | 8-pin DIP socket, 0.3 inch | For the amplifier |
| 1 | 10 kohm | R1. EVENT pull-up |
| 1 | 10 kohm | R2. DAC load, from PD6 to GND |
| 1 | 22 kohm | R3. Series resistor into the LM386 |
| 1 | 1 kohm | R4. Shunt at the LM386 input |
| 1 | 10 ohm | R5. Zobel resistor at the LM386 output |
| 1 | 100 uF electrolytic | C5. LM386 supply, from pin 6 to pin 4 |
| 1 | 220 uF electrolytic | C2. LM386 output coupling to the speaker |
| 1 | 1 uF electrolytic | C1. DAC coupling into the divider. Positive faces the DAC |
| 1 | 47 nF | C3. Zobel capacitor, in series with the 10 ohm |
| 1 | 100 nF | C4. Audio AVR VDD, pin 20, to GND pin 21 |
| 1 | 100 nF | C6. Audio AVR AVDD, pin 14, to GND pin 15 |
| 1 | 100 nF | C7. Audio AVR VDDIO2, pin 6, to GND pin 21 |
| 1 | 100 nF | C8. LM386 supply, from pin 6 to pin 4 |
| 1 | 10 uF electrolytic | C9. LM386 pin 7 to pin 4 |
| 7 | Net tie | Copper short from a bypass net to the 5 V rail or to ground |

## Signals

| Signal | Package pin | Port | Direction | Notes |
| --- | --- | --- | --- | --- |
| EVENT | 23 | PA1 | in | From cart contact B1. Idle high |
| DAC | 12 | PD6 | out | DAC0. 10 kohm to GND |
| RESET | 18 | PF6 | reset | Same net as J3 and cart contact B6 |
| UPDI | 19 | UPDI | program | J5 |

J4 is a 2x2 pin header, 2.54 mm pitch. Pin 1 is the square pad. Pins 1 and 4 are GND. Pins 2 and 3 are the speaker, after the 220 uF. Any two adjacent pins are the speaker and ground.

| Pin | Net |
| --- | --- |
| 1 | GND |
| 2 | Speaker, after the 220 uF |
| 3 | Speaker, after the 220 uF |
| 4 | GND |

J5 is a 1x3 pin header, 2.54 mm pitch. Pin 1 is the square pad. The board is powered from the barrel jack during programming.

| Pin | Net |
| --- | --- |
| 1 | UPDI |
| 2 | +5 V |
| 3 | GND |
