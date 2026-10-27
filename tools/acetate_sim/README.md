# Acetate Simulator (`acetate_sim`)

Interactive desktop tool for designing and previewing transparent colored acetate overlays for Maki-Nita's 16x16 LED matrix display.

## Controls

### Mouse
- **Left click / drag on grid**: Turn LEDs ON or OFF (toggle on initial click, paint while dragging).
- **Left click on top color stripe**: Select active acetate color (first swatch is fully transparent / clear filter).
- **Right click / right-drag on grid**: Paint acetate color region onto the display overlay.

### Modifier Keys (Hold while clicking / dragging)
- **Hold `F` (Flood Fill)**:
  - **Left Click**: Flood-fills connected region of LEDs (toggling on/off state).
  - **Right Click**: Flood-fills connected region of acetate with the active color.
- **Hold `L` (Straight Lines)**:
  - **Left Drag**: Draws a straight line of LEDs with real-time preview (applies on release).
  - **Right Drag**: Draws a straight line of acetate color with real-time preview (applies on release).

### Keyboard Shortcuts
- `1` - `8`: Select acetate color swatch (1 is transparent, 2-8 are colors).
- `S` or `Ctrl + S`: Take a full app screenshot, copy to clipboard, and save to `acetate_screenshot.png`.
- `C`: Clear all LEDs (turn all off).
- `A`: Clear all acetate (reset overlay to transparent).
- `Q` or `Esc`: Quit.

## Build and Run

From repository root:
```bash
./acetate.sh
```
Or directly:
```bash
make -C tools/acetate_sim
./tools/acetate_sim/acetate-sim
```
