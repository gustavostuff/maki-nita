# Screen overlays and game packaging

The display field is 16x16 and 1-bit. Color and visual tone come from a physical translucent sheet placed in front of white LED matrices, drawing direct inspiration from early tabletop games and the Magnavox Odyssey.

## The physical game kit

Each game is conceived as a complete, tactile physical release rather than just a software binary. Opening a game box reveals a self-contained kit:

- A cartridge in its custom plastic shell, containing the game AVR and ROM.
- A colored translucent screen overlay sheet.
- A snap-on magnetic marquee panel featuring game title artwork.
- A printed mini-manual illustrating controls, scoring, and lore.

Preparing to play involves an enjoyable physical ritual. Inserting the cartridge behind the marquee slot, snapping the matching magnetic marquee into place along the console top, and sliding the colored overlay sheet into the faceplate turns the tabletop console into a dedicated arcade cabinet for that specific title.

## Visual mechanics

The playfield emitters use white light. The overlay is a single translucent sheet (such as tinted acetate, lighting gel film, or color-printed transparency) placed directly over the screen. A single sheet can carry multiple colors across different zones of the 16x16 field:

- A single sheet with multiple color bands (for example, green along the bottom rows, amber in the middle, and red or blue across the top) causes moving white pixels to naturally change color as they traverse the display. This mirrors classic arcade cabinets like Space Invaders and Breakout.
- A solid single-color sheet (red, amber, green, cyan, etc) gives the entire game a unified arcade glow.

Because all color filtering happens mechanically through the optical sheet, game firmware remains fast, light, and 1-bit. Moving sprites change perceived color solely based on which section of the sheet they illuminate. A game runs completely fine with no overlay installed, displaying crisp white pixels, while sliding in an overlay gives each cartridge its own visual identity.

## Materials and fabrication

A single multi-color overlay can be produced by color-printing translucent inks onto clear acetate film, or by assembling strips of colored lighting gel onto a transparent carrier sheet. Solid overlays can be cut directly from colored acetate sheets or polyester gels with a craft knife or laser cutter. The sheet size matches the 16x16 matrix faceplate window (approximately 64x64 mm).
