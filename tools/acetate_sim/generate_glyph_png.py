#!/usr/bin/env python3
"""Generate 1:1 scale PNG images of the Maki-Nita 4x4 glyph font from img/fonts/4x4_font.png.

Black is transparent.
Characters (44 glyphs, 4x4 each):
  0..25:  A-Z
  26..35: 0-9
  36..43: . , : ; - + = /
"""

from pathlib import Path
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parents[2]
IMG_DIR = REPO_ROOT / "img" / "fonts"
FONT_PNG = IMG_DIR / "4x4_font.png"

CHAR_ORDER = [
    # Letters (26)
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    # Digits (10)
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    # Punctuation & math (8)
    ".", ",", ":", ";", "-", "+", "=", "/"
]


def load_glyphs_from_png():
    img = Image.open(FONT_PNG).convert("RGB")
    width, height = img.size
    cell_w = 4
    cell_h = 4
    num_glyphs = width // cell_w

    glyphs = []
    pix = img.load()
    for g_idx in range(num_glyphs):
        glyph_data = []
        x_base = g_idx * cell_w
        for y in range(cell_h):
            row_data = []
            for x in range(cell_w):
                r, g, b = pix[x_base + x, y]
                # White pixel is active; black is transparent
                is_active = (r > 128 or g > 128 or b > 128)
                row_data.append(is_active)
            glyph_data.append(row_data)
        glyphs.append(glyph_data)
    return glyphs


def generate():
    IMG_DIR.mkdir(parents=True, exist_ok=True)
    glyphs = load_glyphs_from_png()
    num_glyphs = len(glyphs)
    print(f"Loaded {num_glyphs} glyphs from {FONT_PNG}")

    cell_w = 4
    cell_h = 4
    pad = 1

    # 4 rows x 11 columns = 44 glyphs
    cols_count = 11
    rows_count = 4

    grid_w = pad + cols_count * (cell_w + pad)
    grid_h = pad + rows_count * (cell_h + pad)

    variants = [
        (IMG_DIR / "glyph_font.png", (0, 0, 0, 0)),
        (IMG_DIR / "glyph_font_dark.png", (10, 10, 12, 255)),
    ]

    for path, bg in variants:
        img = Image.new("RGBA", (grid_w, grid_h), bg)
        pix = img.load()
        for idx, g in enumerate(glyphs):
            r_idx = idx // cols_count
            c_idx = idx % cols_count
            x_start = pad + c_idx * (cell_w + pad)
            y_start = pad + r_idx * (cell_h + pad)
            for row in range(cell_h):
                for col in range(cell_w):
                    if g[row][col]:
                        pix[x_start + col, y_start + row] = (255, 255, 255, 255)

        img.save(path)
        print(f"Wrote {path} ({grid_w}x{grid_h})")

    # Linear horizontal strip of all glyphs (transparent black background)
    strip_w = pad + num_glyphs * (cell_w + pad)
    strip_h = pad + cell_h + pad
    strip_img = Image.new("RGBA", (strip_w, strip_h), (0, 0, 0, 0))
    strip_pix = strip_img.load()
    for c_idx, g in enumerate(glyphs):
        x_start = pad + c_idx * (cell_w + pad)
        for row in range(cell_h):
            for col in range(cell_w):
                if g[row][col]:
                    strip_pix[x_start + col, pad + row] = (255, 255, 255, 255)

    strip_path = IMG_DIR / "glyph_font_strip.png"
    strip_img.save(strip_path)
    print(f"Wrote {strip_path} ({strip_w}x{strip_h})")


if __name__ == "__main__":
    generate()
