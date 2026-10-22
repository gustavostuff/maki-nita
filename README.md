<img src="img/logo_v_03.png" alt="Logo V2" />

<img src="img/sim.png" alt="Simulator" />

Maki Nita is a one-stick, one-button portable tabletop arcade. The name comes from maquinita, the Mexican word generally used for an arcade machine. The display is a 16x16 grid of red 5 mm LEDs. Each pixel is on or off.

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
- [Software](docs/software.md) covers one tick, the framebuffer, the inputs, and what a game is.
- `sim/maki-nita-ui` is the picture of the screen and the controls. `make -C sim` builds it.
