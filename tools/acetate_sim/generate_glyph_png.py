#!/usr/bin/env python3
"""Generate 1:1 scale PNG images of the Maki-Nita glyph font."""

from pathlib import Path
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parents[2]
IMG_DIR = REPO_ROOT / "img"

DIGITS = [
    [0x07, 0x05, 0x05, 0x05, 0x07],  # 0
    [0x03, 0x02, 0x02, 0x02, 0x07],  # 1
    [0x07, 0x04, 0x07, 0x01, 0x07],  # 2
    [0x07, 0x04, 0x06, 0x04, 0x07],  # 3
    [0x05, 0x05, 0x07, 0x04, 0x04],  # 4
    [0x07, 0x01, 0x07, 0x04, 0x07],  # 5
    [0x07, 0x01, 0x07, 0x05, 0x07],  # 6
    [0x07, 0x04, 0x04, 0x04, 0x04],  # 7
    [0x07, 0x05, 0x07, 0x05, 0x07],  # 8
    [0x07, 0x05, 0x07, 0x04, 0x04],  # 9
]

LETTERS = [
    [0x02, 0x05, 0x07, 0x05, 0x05],  # A
    [0x03, 0x05, 0x03, 0x05, 0x03],  # B
    [0x06, 0x01, 0x01, 0x01, 0x06],  # C
    [0x03, 0x05, 0x05, 0x05, 0x03],  # D
    [0x07, 0x01, 0x03, 0x01, 0x07],  # E
    [0x07, 0x01, 0x03, 0x01, 0x01],  # F
    [0x07, 0x01, 0x01, 0x05, 0x07],  # G
    [0x05, 0x05, 0x07, 0x05, 0x05],  # H
    [0x07, 0x02, 0x02, 0x02, 0x07],  # I
    [0x07, 0x02, 0x02, 0x02, 0x01],  # J
    [0x05, 0x05, 0x03, 0x05, 0x05],  # K
    [0x01, 0x01, 0x01, 0x01, 0x07],  # L
    [0x11, 0x1b, 0x15, 0x11, 0x11],  # M
    [0x09, 0x0b, 0x0d, 0x09, 0x09],  # N
    [0x06, 0x09, 0x09, 0x09, 0x06],  # O
    [0x07, 0x09, 0x07, 0x01, 0x01],  # P
    [0x06, 0x09, 0x09, 0x0d, 0x0e],  # Q
    [0x07, 0x09, 0x07, 0x09, 0x09],  # R
    [0x0e, 0x01, 0x06, 0x08, 0x07],  # S
    [0x07, 0x02, 0x02, 0x02, 0x02],  # T
    [0x09, 0x09, 0x09, 0x09, 0x06],  # U
    [0x11, 0x11, 0x11, 0x0a, 0x04],  # V
    [0x11, 0x11, 0x11, 0x15, 0x0a],  # W
    [0x05, 0x05, 0x02, 0x05, 0x05],  # X
    [0x05, 0x05, 0x02, 0x02, 0x02],  # Y
    [0x0f, 0x08, 0x06, 0x03, 0x0f],  # Z
]

PUNCT = [
    [0x00, 0x00, 0x00, 0x00, 0x02],  # .
    [0x00, 0x00, 0x0e, 0x00, 0x00],  # -
    [0x00, 0x02, 0x00, 0x02, 0x00],  # :
]

# 3x5 font used in acetate_sim badge
SIM_COPIED = [
    [0b111, 0b100, 0b100, 0b100, 0b111],  # C
    [0b111, 0b101, 0b101, 0b101, 0b111],  # O
    [0b111, 0b101, 0b111, 0b100, 0b100],  # P
    [0b111, 0b010, 0b010, 0b010, 0b111],  # I
    [0b111, 0b100, 0b110, 0b100, 0b111],  # E
    [0b110, 0b101, 0b101, 0b101, 0b110],  # D
]


def generate():
    IMG_DIR.mkdir(parents=True, exist_ok=True)

    rows = [
        DIGITS + PUNCT,
        LETTERS[:13],
        LETTERS[13:],
    ]

    cols_count = 13
    cell_w = 5
    cell_h = 5
    pad = 1

    w = pad + cols_count * (cell_w + pad)
    h = pad + 4 * (cell_h + pad)

    variants = [
        (IMG_DIR / "glyph_font.png", (0, 0, 0, 0)),
        (IMG_DIR / "glyph_font_dark.png", (10, 10, 12, 255)),
    ]

    for path, bg in variants:
        img = Image.new("RGBA", (w, h), bg)
        pix = img.load()
        for r_idx, r_glyphs in enumerate(rows):
            y_start = pad + r_idx * (cell_h + pad)
            for c_idx, g in enumerate(r_glyphs):
                x_start = pad + c_idx * (cell_w + pad)
                for row in range(5):
                    for col in range(5):
                        if (g[row] >> col) & 1:
                            pix[x_start + col, y_start + row] = (255, 255, 255, 255)

        # 4th row: 3x5 font COPIED
        y_start = pad + 3 * (cell_h + pad)
        for c_idx, g in enumerate(SIM_COPIED):
            x_start = pad + c_idx * (cell_w + pad)
            for row in range(5):
                for col in range(3):
                    if (g[row] >> (2 - col)) & 1:
                        pix[x_start + col, y_start + row] = (255, 255, 255, 255)

        img.save(path)
        print(f"Wrote {path} ({w}x{h})")

    # Horizontal linear strip of all 39 primary glyphs
    all_glyphs = DIGITS + LETTERS + PUNCT
    strip_w = pad + len(all_glyphs) * (cell_w + pad)
    strip_h = pad + cell_h + pad
    strip_img = Image.new("RGBA", (strip_w, strip_h), (0, 0, 0, 0))
    strip_pix = strip_img.load()
    for c_idx, g in enumerate(all_glyphs):
        x_start = pad + c_idx * (cell_w + pad)
        for row in range(5):
            for col in range(5):
                if (g[row] >> col) & 1:
                    strip_pix[x_start + col, pad + row] = (255, 255, 255, 255)
    strip_path = IMG_DIR / "glyph_font_strip.png"
    strip_img.save(strip_path)
    print(f"Wrote {strip_path} ({strip_w}x{strip_h})")


if __name__ == "__main__":
    generate()
