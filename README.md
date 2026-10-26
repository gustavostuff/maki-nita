<img src="img/logo_v_03.png" alt="Logo V2" />

Maki-Nita is a one-stick, one-button portable tabletop arcade. The name comes from _maquinita_, the Mexican word generally used for an arcade machine. The display is four white 8x8 LED matrices in a 16x16 field. Graphics are 1-bit (LEDs are either ON or OFF).

On a similar spirit as the Magnavox Odyssey, Maki-Nita uses colored, translucent sheets to cover the screen. So the overlay is an extension of the cartridge, part of the game experience. A magnetic marquee runs along the top and the cartridge sits behind it. The [overlays](docs/overlays.md) and [design](docs/design.md) docs cover the sheet and the console netlist.

An AVR128DB28 on a cartridge runs the game and holds the picture (the cartridge is both the console brain and the game data). Four MAX7219 drivers refresh the screen grid.

## Goals

Simple graphics can still be fun. A stick (and sometimes a button) is all that's needed. Together with cartridges, colored overlays, and a sturdy design, that is the spirit of Maki-Nita.

The build is a simple soldering job.

- The parts are through-hole, on a 2.54 mm grid.
- The chips sit in sockets, so a bad one can be swapped.
- Wires leave the board on headers. One is the controls. One is power and reset. One is the speaker.

The parts are cheap.

- The console is a 100x100 mm board. The cartridge is a small 2-layer board.
- The screen is four white/neutral-white 8x8 LED matrices (normally less than 1 USD each), and the chips are commonly available parts.

- Power is any generic/regulated 5 V adapter.

Software is under the MIT License. The hardware design is under the CERN Open Hardware Licence Version 2, Strongly Reciprocal.

## Settled choices

- Display: 16x16, four common-cathode 8x8 white or neutral-white matrices, one bit per pixel. Colored [overlays](docs/overlays.md) sit in front of the field.
- Board: a 2-layer console PCB and a 2-layer cartridge, each with a ground pour on both sides. The console carries the matrices, the drivers, the cart socket, the audio AVR, and the amplifier.
- CPU: AVR128DB28-I/SP on the cartridge at 12 MHz. Console audio AVR on the main board at 20 MHz. The console socket is an EDAC 345-016-520-212, 16 contacts, 8 a side.
- Drivers: four MAX7219ENG+ or MAX7219CNG+, one per 8x8 block.
- Power: regulated 5 V barrel jack and a panel switch. The rail and reset enter the console on a 2x2 header.
- Controls: one arcade stick (four switches) and one 30 mm button.
- Sound: a second AVR128DB28 on the console mixes a square wave, a triangle, and noise. The cartridge sends one index byte into a catalog of 256 sounds. An LM386N-1 drives an 8 ohm speaker on a 2x2 header. A game may use music, effects, both, or neither.
- Brightness: on the console, hold the Bright button, then tap stick Up or Down to step brightness. Eight levels only (1, 3, 5, 7, 9, 11, 13, 15), all four MAX7219 drivers. The cart edge is unchanged.
- Marquee: removable magnetic panel. A few extra LEDs backlight the marquee while the console is powered. Simple magnets hold the panel. The playfield is the 256 matrix LEDs only.

## PCBs

Copper is still in progress. The [SKiDL netlist](tools/kicad/skidl/maki_nita.py) and KiCad files under `tools/kicad/main-pcb/` are the electrical reference.

Main system PCB, 100x100 mm:

<img src="img/main_pcb_concept.png" alt="Main PCB concept" />

Cartridge PCB:

<img src="img/cart_pcb_concept.png" alt="Cart PCB concept" />

## Simulator

<img src="img/sim.png" alt="Simulator" />

`tools/sim` is a netlist simulation of the console. The same game C as the cartridge runs on the host and drives that model. AVR `.hex` files are not executed.

SKiDL export writes `tools/kicad/skidl/maki-nita.net` and updates net classes in `tools/kicad/main-pcb/maki-nita.kicad_pro`. See [firmware](docs/firmware.md).

`./sim.sh` opens the UI. The active cart is named in `cart_placed.json`.

## Docs

- [Design](docs/design.md) is the console netlist, cart edge, display chain, and audio wiring from SKiDL and KiCad.
- [Firmware](docs/firmware.md) is the game loop, inputs, sound, SDK, and simulator.
- [Overlays](docs/overlays.md) is the colored sheet in front of the field.
