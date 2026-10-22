<img src="img/logo_v_03.png" alt="Logo V2" />

<img src="img/sim.png" alt="Simulator" />

Maki Nita is a one-stick, one-button portable tabletop arcade. The name comes from maquinita, the Mexican word generally used for an arcade machine. The display is four red 8x8 matrices on one board, set as a 16x16 field. Each pixel is on or off.

An AVR128DB28 on a cartridge runs the game and holds the picture. Four MAX7219 drivers on the console refresh the grid.

## Settled choices

- Display: 16x16, four common-cathode 8x8 matrices, one bit per pixel.
- Board: a console PCB and a cartridge. The matrices are on top of the console. The drivers and the cart socket sit in a strip below them.
- CPU: AVR128DB28-I/SP on the cartridge, internal 24 MHz clock. The console socket is an EDAC 345-016-520-212, 16 contacts, 8 a side.
- Drivers: four MAX7219ENG+ or MAX7219CNG+, one per 8x8 block.
- Power: regulated 5 V barrel jack and a panel switch.
- Controls: one arcade stick (four switches) and one 30 mm button.

## Docs

- [Hardware](docs/hardware.md) covers the board, the parts, the drivers, power, and headers.
- [Software](docs/software.md) covers one tick, the framebuffer, the inputs, and what a game is.
- `sim/maki-nita-ui` is the picture of the screen and the controls. `make -C sim` builds it.
