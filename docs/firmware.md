# Firmware and simulator

Game firmware runs on the cartridge AVR at 12 MHz. Audio firmware runs on the console AVR at 20 MHz. Sources are under `tools/fw/`. The simulator in `tools/sim/` runs the same game C on the host against a netlist model of the console. Firmware and the simulator are under the MIT License.

## Game loop

The runtime loops at 60 Hz. Each pass debounces inputs, calls `game_tick`, then flushes the frame to the MAX7219 chain.

The framebuffer is sixteen rows of sixteen bits in `frame[16]`. `pixel`, `pixel_get`, and `clear` are the drawing API. There is no per-pixel brightness in RAM. Global intensity is set on the console (see below).

DIN is PA4, CLK is PA6, LOAD is PA7. One flush sends eight digit rows across all four drivers. Init leaves decode off, scan limit at all digits, and intensity at full scale until the console changes it. Details match `tools/fw/flush.c`.

## Inputs and debouncing

Stick and action button nets reach PD1 through PD5 on the cart. Each switch is active low with an internal pull-up.

Every switch uses the same rule in `tools/fw/input.c`: one sample per 60 Hz tick, two consecutive disagreeing ticks before stable changes, then a pressed or released edge. That is about 33 ms.

The planned console brightness control uses the same debouncing on a Bright button and on Up and Down while Bright is held. Levels are 1, 3, 5, 7, 9, 11, 13, and 15 on all four MAX7219 parts. Zero is not used. Bright and intensity writes are not in the SKiDL netlist yet. The simulator does not model Brightness.

## Sound

Games call `sound(index)` with a catalog index 0 through 255. Index 0 is silent. Filled rows live in `tools/fw/synth.c`. The cart queues bytes on EVENT. The console AVR decodes them and mixes square, triangle, and noise at 20 kHz.

## Entity SDK

Games may use `tools/fw/entity.c` for moving shapes. Up to 64 slots. A 32 by 32 grid wraps the screen with a one-screen margin. `world_draw` copies the center 16 by 16 into `frame`. Shapes are up to 4 by 4, packed with `SHAPE1` through `SHAPE4`. `entity_spawn`, `entity_step`, `world_tick`, and `world_pairs` cover motion and collision. See `tools/fw/entity.h`.

## Carts

Each cart implements `void game_tick(const Input *in)`. Names in `cart_placed.json` are `space-shooter`, `paddle`, and `arkaled` (test games so far). Build with `make -C tools/fw/avr` or `CART=paddle` / `CART=arkaled`. Console audio is `make -C tools/fw/audio`.

Game rules and scoring live in `games/` and in the cart sources, not in this doc.

## Simulator

`./sim.sh` builds `tools/sim/maki-nita-ui`. The netlist model ties cart pins to U1 to U4, EVENT, and J4. Host code runs at 60 Hz with 12 MHz and 20 MHz timing constants in `tools/sim/clocks.h`. AVR `.hex` files are not executed. Brightness stays at the flush default.
