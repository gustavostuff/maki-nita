# Screen overlays and game packaging

The display field is 16x16 and 1-bit. Color, scenery, and visual context come from a physical translucent sheet placed in front of white or neutral-white LED matrices, drawing direct inspiration from early tabletop games and the Magnavox Odyssey.

## The physical game kit

Each game is conceived as a complete, tactile physical release rather than just a software binary. Opening a game box reveals a self-contained kit:

- A cartridge in its custom plastic shell, containing the game AVR and ROM.
- A printed translucent screen overlay tailored to the playfield art.
- A snap-on magnetic marquee panel featuring game title artwork.
- A printed mini-manual illustrating controls, scoring, lore, and artwork.

Preparing to play involves an enjoyable physical ritual. Inserting the cartridge behind the marquee slot, snapping the matching magnetic marquee into place along the console top, and sliding the colored overlay sheet into the faceplate turns the tabletop console into a dedicated arcade cabinet for that specific title.

## Visual mechanics

The playfield emitters use broad-spectrum white light. The overlay consists of an opaque black mask printed with vibrant, translucent color windows. When individual white LEDs light up beneath the sheet, they bring the printed illustrations to life:

- Translucent red, amber, green, cyan, or magenta regions turn monochrome 1-bit dots into colored spaceships, glowing lasers, shields, and bouncing balls.
- Opaque regions create playfield borders, obstacles, goal zones, and score counters.
- Ambient room light gently illuminates the background artwork on the sheet, while the LEDs shine through from behind with high contrast.

Because all color filtering happens mechanically in the sheet, game firmware remains fast, light, and 1-bit. A game runs completely fine with no overlay installed, but slotting the sheet completes the visual experience.

## Artwork and templates

Overlays use a shared 16x16 coordinate grid matching the 2.54 mm LED matrix spacing. Game creators can print sheets on standard transparency film, laser cut them, or produce full-color litho prints with opaque white backing layers for rich color saturation.
