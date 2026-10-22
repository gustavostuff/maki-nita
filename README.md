<img src="img/logo/logo_v_01.png" alt="RetroDiode logo v01" />

RetroDiode is a one-stick, one-button portable tabletop arcade. The display is a 16x16 grid of red 5 mm LEDs. Each pixel is on or off. The first games are small ones such as Tetris and a space shooter.

An AVR128DB28 runs the game and holds the picture. Four MAX7219 drivers share the same board, in a strip under the LEDs, and refresh the grid.

## Settled choices

- Display: 16x16, 5 mm red, one bit per pixel. Pitch comes later.
- Board: one PCB. The LED field is on top. The drivers and the CPU sit in a strip below it.
- CPU: AVR128DB28-I/SP, in a socket, internal 24 MHz clock.
- Drivers: four MAX7219ENG+, one per 8x8 block.
- Power: regulated 5 V barrel jack and a panel switch.
- Controls: one arcade stick (four switches) and one 30 mm button.

## Docs

- [Hardware](docs/hardware.md) covers the board, the parts, the drivers, power, and headers.
- [Software](docs/software.md) covers the framebuffer, the driver update, the inputs, and what a game is.
- A picture of the screen and the controls is `sim/retrodiode-ui`, built with `make -C sim`. The LEDs in that picture come from the four MAX7219 models. The same shooter runs there and in `fw/avr/game.hex`. The canvas is 128 by 192 pixels. The stick follows WASD or the arrow keys and steps the ship one LED every four ticks. Space fires a shot upward. Esc closes the window. `make -C sim test` checks each chip, the chain, and that game. From the repo root, `./build_all.sh` builds the picture and runs those checks.
