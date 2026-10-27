<img src="img/logo_v_03.png" alt="Logo V2" />

Maki-Nita is a cartridge-based, two-player portable tabletop Arcade. The name comes from _maquinita_, the Mexican word generally used for an arcade machine. The display is four white 8x8 LED matrices in a 16x16 field. Graphics are 1-bit (LEDs are either ON or OFF).

On a similar spirit as the Magnavox Odyssey, Maki-Nita uses colored, translucent sheets to cover the screen. So the overlay is an extension of the cartridge, part of the game experience. A magnetic marquee runs along the top and the cartridge sits behind it. The [overlays](docs/overlays.md) and [design](docs/design.md) docs cover the sheet and the console netlist.

An AVR128DB28 on a cartridge runs the game and holds the picture (the cartridge is both the console brain and the game data). Four MAX7219 drivers refresh the screen grid.

## Goals

Simple graphics can still be fun. A stick (and sometimes a button) is all that's needed. Together with cartridges, colored overlays, and a sturdy design, that is the spirit of Maki-Nita.

The build is a simple soldering job.

- The parts are through-hole, on a 2.54 mm grid.
- The chips sit in sockets, so a bad one can be swapped.
- Wires leave the board on headers. Two for player controls, another one for power/reset, and one more for the speaker.

The parts are cheap.

- The console is a 100x100 mm board. The cartridge is an even smaller, two-layer board.
- The screen is four white 8x8 LED matrices (normally less than 1 USD each), and the chips are commonly available parts.

- Power is any generic/regulated 5 V adapter.

Software is under the MIT License. The hardware design is under the CERN Open Hardware Licence Version 2, Strongly Reciprocal.

## Settled choices

- Display: 16x16, four common-cathode 8x8 white matrices, one bit per pixel. Colored [overlays](docs/overlays.md) sit in front of the field.
- Board: a 2-layer console PCB and a 2-layer cartridge, each with a ground pour on both sides. The console carries the matrices, the drivers, the cart socket, the audio AVR, and the audio amplifier.
- CPU: AVR128DB28-I/SP on the cartridge at 12 MHz. Console audio AVR on the main board at 20 MHz. The console socket is an EDAC 395-020-520-202, 20 contacts, 10 a side.
- Drivers: four MAX7219ENG+ or MAX7219CNG+, one per 8x8 block.
- Power: regulated 5 V barrel jack and a panel switch. The rail and reset enter the console on a 2x2 header.
- Controls: two arcade sticks (four switches each) and two 30 mm buttons, with one stick and one button per player. Player 1 controls are built into the cabinet. Because the cabinet is small, the second player controller connects on the side. Each player connects through a 2x3 header (four directions, button, and GND).
- Sound: a second AVR128DB28 on the console mixes a square wave, a triangle, and noise. The cartridge sends one index byte into a catalog of 256 sounds. An LM386N-1 drives an 8 ohm speaker on a 2x2 header. A game may use music, effects, both, or neither.
- Brightness: tap the Bright button to step brightness up to the next level (wrapping around to minimum). Holding the Bright button while tapping stick Up or Down also steps through the eight levels (1, 3, 5, 7, 9, 11, 13, 15) on all four MAX7219 drivers.
- Marquee: removable magnetic panel along the top, with game-specific art.

## PCBs

Copper tracing is pretty much done but still needs to be double-checked (EMI considerations, noise, test points, etc). The [SKiDL netlist](tools/kicad/skidl/maki_nita.py) and KiCad files are under `tools/kicad/main-pcb/` and `tools/kicad/cart-pcb/`.  

Main system PCB, 100x100 mm:

<img src="img/main_pcb_concept.png" alt="Main PCB concept" />

Cartridge PCB, 44x37 mm:

<img src="img/cart_pcb_concept.png" alt="Cart PCB concept" />

## Simulator

<img src="img/sim.png" alt="Simulator" />

`tools/sim` is a netlist simulation of the console. The simulator compiles the cartridge's C game code natively on the host to drive the hardware model. This same C code can be directly flashed to the AVR on the cartridge.

SKiDL export writes `tools/kicad/skidl/maki-nita.net` and updates net classes in `tools/kicad/main-pcb/maki-nita.kicad_pro`. See [firmware](docs/firmware.md).

`./sim.sh` opens the UI. The active cart is named in `cart_placed.json`.

## Docs

- [Design](docs/design.md) is the console netlist, cart edge, display chain, and audio wiring from SKiDL and KiCad.
- [Firmware](docs/firmware.md) is the game loop, inputs, sound, SDK, and simulator.
- [Overlays](docs/overlays.md) is the colored sheet in front of the field.

## Future plans

Once the base design has been proven/tested, these are the next things to look at:

* Translucent art for the marquees and backlight LEDs to make it shine, as in real classic arcades.
* Better audio quality by using the AKWF (Adventure Kid Waveforms) library. Samples would live in the Audio AVR and be triggered via the already defined EVENT signal.
* USB C power port as an alternative to the barrel (maybe a switch to pick one or the other, or priority-based).
* In the long run, maybe a "high-res" version of the Maki-Nita system (16 matrices, 32x32 screen). It could be retro-compatible with 16x16 games, by doubling pixels to 2x2 regions.

