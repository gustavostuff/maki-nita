# Screen overlays

The 16x16 field is 1-bit. Every LED is on or off. Color, scenery, and game identity come from a physical sheet in front of the matrices, without extra silicon or a wider framebuffer. That sheet belongs to the same design as cartridges. Firmware and the faceplate assume white light on the field and a replaceable overlay. A title runs with no sheet installed.

The field uses white or neutral-white LEDs only. Single-color matrix blocks cannot carry color in the sheet. White light passes through translucent windows in the mask. Fixed-color emitters paint in the LED instead of in the plastic.

Magnavox shipped the same idea with the Odyssey (1972). That console drew a few white spots on a black CRT. Thin translucent plastic overlays turned those spots into tennis courts, ski slopes, and haunted houses. The electronics stayed simple. The sheet supplied the world.

Maki-Nita uses the same trick on a 64 mm square LED field.

The console, the cartridge, and the rest of the hardware design are under the CERN Open Hardware Licence Version 2, Strongly Reciprocal.

## How it works

The playfield uses white or neutral-white 8x8 matrices. A thin black mask carries translucent colored windows. White light passes only through those openings. Opaque areas stay dark. Firmware treats each pixel as on or off. The framebuffer and the MAX7219 chain stay the same.

```mermaid
flowchart LR
  fw[1-bit framebuffer] --> drivers[MAX7219]
  drivers --> white[White LEDs]
  white --> mask[Colored overlay]
  mask --> view[View]
```

The hardware doc covers the cabinet, the marquee, and playfield backlight.

## Why overlays fit the design

Graphics stay simple in code. Color and backdrop live in plastic or film. A cart ships with or recommends a matching sheet. Cart plus sheet marks a title the way Magnavox paired overlays with Odyssey games. Sheets are cheap to cut, print, or laser. A mechanical template is enough for third-party sheets without a firmware change.

Swapping an overlay is a physical step beside inserting a cartridge. That ritual belongs beside cartridge insertion. Each game reads as a different place. Community and artist sheets sit beside official ones.

## Construction

The field is small, so alignment is forgiving. A recess, a few pins, magnets, or a keyed faceplate keeps the sheet flat and square.

Typical stacks:

- Black PET, thin acrylic, or card with laser-cut openings and colored gel or tinted acrylic inserts.
- Printed transparency between clear laminates, with solid black ink where light must not pass.
- A replaceable insert held by a bezel that also protects the LEDs.

White or cool-white LEDs give the cleanest filter colors. Warm white suits amber or red scenes. A light diffuser in the stack softens the dot grid.

Brightness may need tuning. White at the same peak current reads brighter than a single-color matrix. The MAX7219 intensity register or the ISET resistor can be adjusted, or the overlay can diffuse the beam.

## Overlay styles

Full-scene sheets color the whole field, such as court lines, water, dungeon walls, or a starfield with colored planets.

Zone sheets leave fixed colored bands (score strip, danger columns, status row). Games can assume those zones even though the framebuffer stays 1-bit.

Window sheets use rounded or organic openings over part of the grid so the picture reads less strictly square.

Cart-linked sheets pair one overlay with one game. Cart plus sheet together mark a new title.

Community sheets need only a published mechanical template (pixel grid, outer size, alignment features). SVG or PDF files are enough for print, vinyl, or laser work.

## Mechanical template

Exact outer dimensions, the 16x16 pixel grid over the 64 mm field, and alignment features are open. That template matches the faceplate or recess in the cabinet. The hardware doc covers the marquee and the cart bay.
