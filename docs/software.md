# Maki-Nita software

The cartridge AVR holds the picture and reads the controls. The console AVR holds the sound catalog and plays it. The four MAX7219 drivers store the blocks and scan them. A game never talks to a driver, a pin, or the audio AVR.

Both AVRs run plain avr-gcc from the internal 24 MHz oscillator. Each chip is programmed over its own UPDI pin. The firmware and the simulator are under the MIT License.

## One tick

The runtime loops at 60 Hz. Each pass reads the controls, calls the game, then flushes the frame.

Up, down, left, right, and the button are PD1, PD2, PD3, PD4, and PD5. A closed switch reads low. The input doc lists the package pins. Debounce is two ticks, described below.

The flush leaves on three port A pins. PA4 is DIN, PA6 is CLK, and PA7 is LOAD. The four MAX7219 drivers take that serial stream, latch one 8x8 block each, and scan the LEDs on their own. The bit order is in the flush section below.

That loop is the same C in the simulator and in the AVR image. The simulator drives a model of those pins and reads the model back. The AVR image reads PORTD and drives PORTA. The simulator does not run AVR instructions. It plays the same catalog as the console image. EVENT is a net on the board model, from game AVR pin 22 to audio AVR pin 23, with a pull-up. The simulator drives that net with the cartridge's 100 us and 200 us pulses and samples it at 40 kHz. SDL plays each index that comes off the net. `make -C tools/fw/avr` builds `tools/fw/avr/game.hex` for the space-shooter cart. `make -C tools/fw/avr CART=paddle` builds the paddle cart. `make -C tools/fw/audio` builds `tools/fw/audio/audio.hex` for the console chip. The simulator reads `cart_placed.json` and runs that cart. A missing file, a missing `cart` key, or an unknown name leaves the drivers in reset.

## Framebuffer

Sixteen rows, sixteen columns, one bit each. Thirty-two bytes.

```c
uint16_t frame[16];
```

`frame[0]` is the top row. Bit 0 of a row is the leftmost pixel. A set bit is on.

```c
void pixel(uint8_t x, uint8_t y, bool on);
bool pixel_get(uint8_t x, uint8_t y);
void clear(void);
```

`x` and `y` run from 0 to 15. A coordinate outside that range is ignored.

This is the whole display interface. There is no per-pixel brightness, no hardware sprite, and no second layer. Driver intensity stays at full scale.

## Timing

The game runs at 60 Hz. One tick reads the controls, updates the game, and writes pixels. After the tick, the runtime flushes `frame` to the drivers.

Each MAX7219 keeps scanning its block from the last data it latched. A slow tick drops game frames. The LEDs keep showing the last flush.

## Flush

DIN is PA4, CLK is PA6, and LOAD is PA7. CLK idles low. LOAD idles high. Data is sampled on the rising edge of CLK. A word is 16 bits, MSB first. The high byte is the register address. The upper nibble of that byte is unused. The low byte is the data.

For the MAX7219, CLK shifts data regardless of LOAD. The rising edge of LOAD latches the 16 bits then sitting in each driver's shift register. That edge comes after the 64th rising edge of CLK and before any further rising edge. The next rising edge of CLK waits at least 50 ns after LOAD has gone high. LOAD stays high for at least 50 ns.

Four words go out before LOAD rises. The first word reaches the bottom-right driver. The last word reaches the top-left driver. Between those, the words follow the chain in reverse. The chain itself runs left to right, then down a row.

A row of eight pixels becomes one byte with the leftmost pixel in bit 7. That matches the segment wiring in the video doc.

Digit address for block row `d` (0 at the top of the block) is `d + 1`, from 0x01 through 0x08. One transfer writes that same row inside every block. Screen row `br * 8 + d` feeds the two blocks of block row `br`.

One flush sends all eight digit addresses. For each `d` from 0 to 7:

1. LOAD goes low.
2. Four words go out, last driver first. Block `b` takes screen row `(b / 2) * 8 + d`, columns `(b % 2) * 8` through `(b % 2) * 8 + 7`.
3. LOAD goes high.

Before the first flush, the drivers are taken out of the power-up shutdown state.

| Register | Address | Value | Effect |
| --- | --- | --- | --- |
| Display test | 0x0F | 0x00 | Test off |
| Decode mode | 0x09 | 0x00 | Raw segment bits |
| Intensity | 0x0A | 0x0F | Full scale, 31/32 |
| Scan limit | 0x0B | 0x07 | All eight digits |
| Digit 0 to 7 | 0x01 to 0x08 | 0x00 | Blocks cleared |
| Shutdown | 0x0C | 0x01 | Drivers leave shutdown |

Each of those writes is the same four-word transfer, with the same value sent to every driver.

## Controls

Five switches, active low, internal pull-ups. The input layer reports them independently. A diagonal is left and up held together, and the game decides whether that means anything. A 4-way game can ignore a tick where both axes are held, or keep the axis that changed most recently.

Debounce is two game ticks. A switch changes state after the new raw level has lasted through two ticks in a row.

Once per tick the input layer fills this in. `pressed` and `released` are true for that tick only.

```c
typedef struct {
    bool up, down, left, right, button;             /* held */
    bool up_pressed, down_pressed, left_pressed,
         right_pressed, button_pressed;
    bool up_released, down_released, left_released,
         right_released, button_released;
} Input;
```

## Sound

The console AVR holds a catalog of 256 sounds. The cartridge sends one byte on EVENT. The byte is the index, from 0 through 255. The notes stay in the console image. The game image only chooses when to send an index. The audio doc lists the pin, the pulses, and the amplifier.

A game calls one function.

```c
void sound(uint8_t index);
```

The runtime drives PA0. The call does not wait for the pulses. A timer sends the byte, so the 60 Hz tick keeps its pace. The sender keeps a short queue, so two calls in one tick both go out. A game that never calls `sound` leaves the line idle. The speaker stays quiet.

EVENT idles high. The byte goes out least significant bit first, as eight low pulses. A 0 holds the line low for 100 us. A 1 holds it low for 200 us. The line returns high for 100 us between pulses. The audio AVR samples that line at 40 kHz and times each pulse itself. The two chips share no baud rate. One byte lasts about 2 ms, and the line then sits high. The mix is written to the DAC at 20 kHz.

Each filled index is one row. An index with no row is ignored. An index keeps its number when a row is added. The next sound takes the next free index. Index 0 is free.

The three channels have fixed waveforms. One is a square wave, one is a triangle, and one is noise. A row names one channel, whether the phrase ends or holds, and a list of steps. A step is a period, a level, and a duration. The period and the duration count samples at 20 kHz. For the noise channel the period is how often a new random bit is drawn. The level runs from 0 to 255. A row does not label its channel as music or as an effect.

A later index whose row uses the same channel replaces the phrase on that channel. The other channels keep going. A phrase that ends leaves its channel quiet. A phrase that holds stays until another row takes that channel. Level 0 is a silent phrase, and sending that index cuts the channel.

The three channels add into one mix, centered on DAC midscale. A quiet channel adds nothing. Three channels at level 255 together are the full mix. The audio doc sizes the amplifier for that mix.

How a game spends the channels is up to the indexes it sends. A held square and a held triangle leave the noise channel for short phrases. A game may send only phrases that end. A game may send nothing. The console has no mode for those choices.

The rows live in `tools/fw/synth.c`. The console image and the simulator both play that file. In the simulator the index reaches that file across the EVENT net.

The shooter uses two rows. Index 1 is the laser. It plays on the triangle when a bullet is spawned, and the phrase ends. The period starts short and the level starts high, then both fall. Index 2 is the explosion. It plays on the noise channel when a bullet removes a rock, and the phrase ends. The noise starts dense, then the period grows and the level falls. The square channel has no shooter row. The next sound takes index 3.

| Index | Moment | Phrase | Channel |
| --- | --- | --- | --- |
| 1 | A bullet is spawned | ends | triangle |
| 2 | A bullet removes a rock | ends | noise |

## Rocks

The first game is a shooter. The ship sits at the bottom. Its shape points up.

```
010
111
111
```

The stick steps the ship one LED every two ticks, including diagonals. Opposite switches on one axis cancel. The ship stops at the edges of the field.

Rocks are solid 2x2 and 3x3 blocks. They enter from above the field and drift down. Each rock starts by moving one row every 4, 8, 12, or 16 ticks. Over one minute of the round that period shortens to one tick. The faster rocks reach one row every two ticks partway through the minute, and by the end every rock falls once a tick. `ROCK_RAMP` is that minute in 60 Hz ticks. A rock may also step one column every 15 or 30 ticks, to the left, to the right, or not at all. A rock that reaches a side edge stays on the field. The action button fires one bullet. The bullet is a single LED and climbs one cell every two ticks. A bullet that meets a rock removes both and adds one to the score. The score stops at 999. A rock that leaves the bottom of the field is gone. A rock that meets the ship ends the round. The field then shows a face and the round's score in digits. The round's score is compared with the saved score. A higher score replaces the saved score and shows a wide smile. The saved score is one 16-bit word at the start of the cartridge EEPROM. An erased word reads as no high score. The simulator keeps the 512-byte EEPROM image in tools/sim/maki-nita.eep. The same score shows a straight mouth. A score of 999 against a saved 999 also shows the wide smile. A lower score shows a small frown. The digits start at column 1, row 8. The picture stays for one second, and a press during that second does nothing. After that second, the action button starts the next round. The new round begins with the score at zero, and the saved score stays.

Games share a set of 5 by 5 glyphs. The set holds the digits, the letters A to Z, a dot, a dash, and a colon. A lowercase letter uses the capital. `text_draw` paints one line from a chosen column and row. Each glyph takes the next five columns.

The tick stays at 60 Hz. A spawned bullet sends index 1. A bullet that removes a rock sends index 2.

## Paddle

Paddle is the other cart. One paddle stands on the field, one column wide and four LEDs tall. It starts on rows 6 through 9, in column 0. The ball is one LED. It starts at column 8, row 8, and waits there until the paddle moves.

The stick moves the paddle one LED every two ticks. Up and down are the only directions that count. The action button does not move the paddle. A paddle that reaches the top or the bottom stays there until its direction points back onto the field.

On the tick the paddle first moves, the ball steps toward one of the four corners. Steps after that one wait 7 ticks. Every 10 seconds of the rally that wait shrinks by one tick. At one tick per step it stays there. `paddle_period` is that wait. `SLOW_BALL` in the paddle source is a debug switch. While it is 1, those later steps wait 30 ticks instead, which is 2 FPS. The top, bottom, and right edges reverse the ball. A ball alongside the paddle reverses its horizontal step and keeps its vertical step. A ball that meets an end from outside the paddle, or that leaves past an end, reverses both steps. Either bounce adds one to the round's score. The score stays off the field during the rally. The score stops at 999.

A ball that gets past the paddle ends the round. The field then shows the same face and digits as the shooter, and the same comparison with the saved score. The picture stays for one second. After that second, the action button starts the next round. The new round begins with the score at zero and the ball back at the center. The simulator keeps this cart's high score in tools/sim/paddle.eep.

## What a game is

A game is one function called from the 60 Hz tick.

```c
void game_tick(const Input *in);
```

It reads `in`, updates its own static state, and draws by calling `pixel` and `clear`. It may call `sound` with a catalog index. No heap. The 16 KB of RAM holds a piece queue, a small enemy list, and the 32-byte frame.

`main` configures the pins, initializes the drivers, clears the frame, and then loops on the 60 Hz tick. The flush runs after `game_tick` returns. Each cartridge image calls its own game. The names in `cart_placed.json` are `space-shooter` and `paddle`.

## Left out of this contract

Saved data beyond the high score, and a menu on the console for more than one game.
