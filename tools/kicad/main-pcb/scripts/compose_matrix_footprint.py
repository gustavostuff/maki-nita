#!/usr/bin/env python3
"""Compose Matrix8x8 from two standard PinSocket_1x08 footprints.

KiCad cannot nest footprints inside a .kicad_mod. This script is the
source of truth: it copies pads (and optional socket graphics) from the
stock PinSocket and writes the MakiNita matrix footprint.

Edit Matrix8x8_2x8_P2.52mm_R25mm.compose.json, then run this script or
tools/kicad/main-pcb/skidl/export_netlist.sh.
"""

from __future__ import annotations

import json
import re
import uuid
from pathlib import Path

MAIN_PCB = Path(__file__).resolve().parents[1]
SPEC = MAIN_PCB / "library/MakiNita.pretty/Matrix8x8_2x8_P2.52mm_R25mm.compose.json"
OUT = MAIN_PCB / "library/MakiNita.pretty/Matrix8x8_2x8_P2.52mm_R25mm.kicad_mod"
PINSOCK_DIR = Path("/usr/share/kicad/footprints/Connector_PinSocket_2.54mm.pretty")
PINSOCK_STEP = (
    "${KICAD10_3DMODEL_DIR}/Connector_PinSocket_2.54mm.3dshapes/"
    "PinSocket_1x08_P2.54mm_Vertical.step"
)
MATRIX_WRL = "${KIPRJMOD}/library/MakiNita.3dshapes/NFM-12881xx.wrl"
PITCH = 2.54
# PinSocket footprint: pad 1 at y=0, pad 8 at y=17.78 mm. In the composed
# matrix, model offset must match pad 1 at (ox, oy). The Vertical STEP mesh
# puts the pin row 13.97 mm from the model origin (half span + two pitches).
PINSOCK_MODEL_Y_FROM_PAD1 = -(3.5 * PITCH + 2 * PITCH)


def uid() -> str:
    return str(uuid.uuid4())


def load_pin_socket(name: str) -> str:
    path = PINSOCK_DIR / f"{name}.kicad_mod"
    if not path.is_file():
        raise SystemExit(f"PinSocket not found: {path}")
    return path.read_text(encoding="utf-8")


def extract_pads(text: str) -> list[str]:
    return re.findall(r'\t\(pad "\d" thru_hole.*?\n\t\)', text, flags=re.DOTALL)


def pad_number(block: str) -> str:
    m = re.search(r'\(pad "(\d+)"', block)
    if not m:
        raise ValueError(f"no pad number in {block[:40]!r}")
    return m.group(1)


def transform_pad(block: str, ox: float, oy: float, new_num: str) -> str:
    block = re.sub(r'\(pad "\d+"', f'(pad "{new_num}"', block, count=1)

    def at_repl(m: re.Match[str]) -> str:
        x, y = float(m.group(1)), float(m.group(2))
        rot = m.group(3) or "0"
        return f"(at {x + ox} {y + oy} {rot})"

    block = re.sub(r"\(at ([-\d.]+) ([-\d.]+)(?: (\d+))?\)", at_repl, block, count=1)
    if "(uuid " not in block:
        block = re.sub(
            r"\(remove_unused_layers no\)\n\t\)",
            f'(remove_unused_layers no)\n\t\t(uuid "{uid()}")\n\t)',
            block,
            count=1,
        )
    return block


def transform_line(block: str, ox: float, oy: float) -> str:
    def start_repl(m: re.Match[str]) -> str:
        return f"(start {float(m.group(1)) + ox} {float(m.group(2)) + oy})"

    def end_repl(m: re.Match[str]) -> str:
        return f"(end {float(m.group(1)) + ox} {float(m.group(2)) + oy})"

    block = re.sub(r"\(start ([-\d.]+) ([-\d.]+)\)", start_repl, block)
    block = re.sub(r"\(end ([-\d.]+) ([-\d.]+)\)", end_repl, block)
    if "(uuid " not in block:
        block = block.rstrip()
        if block.endswith(")"):
            block = block[:-1] + f'\n\t\t(uuid "{uid()}")\n\t)'
    return block


def extract_silk(text: str) -> list[str]:
    blocks = re.findall(r"\t\(fp_line\n(?:\t.*\n)*?\t\)", text)
    return [b for b in blocks if '(layer "F.SilkS")' in b or '(layer "F.CrtYd")' in b]


def model_block(ox: float, oy: float, y_from_pad1: float) -> str:
    my = oy + y_from_pad1
    return f"""\t(model "{PINSOCK_STEP}"
\t\t(offset
\t\t\t(xyz {ox} {my} 0)
\t\t)
\t\t(scale
\t\t\t(xyz 1 1 1)
\t\t)
\t\t(rotate
\t\t\t(xyz 0 0 0)
\t\t)
\t)"""


def matrix_body_model_block() -> str:
    return f"""\t(model "{MATRIX_WRL}"
\t\t(offset
\t\t\t(xyz 0 0 0)
\t\t)
\t\t(scale
\t\t\t(xyz 1 1 1)
\t\t)
\t\t(rotate
\t\t\t(xyz 0 0 0)
\t\t)
\t)"""


def main() -> None:
    spec = json.loads(SPEC.read_text(encoding="utf-8"))
    y_from_pad1 = float(spec.get("model_y_from_pad1_mm", PINSOCK_MODEL_Y_FROM_PAD1))
    sock_name = spec["pin_socket_name"]
    body = float(spec.get("body_mm", 32))
    sock_text = load_pin_socket(sock_name)
    pads_by_num = {pad_number(p): p for p in extract_pads(sock_text)}

    out_pads: list[str] = []
    out_lines: list[str] = []
    models: list[str] = []

    for strip in spec["strips"]:
        ox, oy = strip["at"]
        pad_map = strip["pad_map"]
        for sock_n, fp_n in zip("12345678", pad_map, strict=True):
            src = pads_by_num.get(sock_n)
            if src is None:
                raise SystemExit(f"PinSocket missing pad {sock_n}")
            out_pads.append(transform_pad(src, ox, oy, str(fp_n)))
        for line in extract_silk(sock_text):
            out_lines.append(transform_line(line, ox, oy))
        models.append(model_block(ox, oy, y_from_pad1))

    fp_name = spec["footprint"]
    lib = spec["pin_socket_lib"]
    descr = (
        f"8x8 LED matrix, {body:.0f} mm body. Composed from two "
        f"{lib}:{sock_name} (see {SPEC.name})."
    )

    content = f"""(footprint "{fp_name}"
\t(version 20260206)
\t(generator "maki-nita-compose")
\t(generator_version "1")
\t(layer "F.Cu")
\t(descr "{descr}")
\t(tags "THT LED matrix 8x8 2.54mm PinSocket composite")
\t(property "Reference" "REF**"
\t\t(at {body / 2} {body / 2 - 9} 0)
\t\t(layer "F.SilkS")
\t\t(uuid "{uid()}")
\t\t(effects
\t\t\t(font
\t\t\t\t(size 1 1)
\t\t\t\t(thickness 0.15)
\t\t\t)
\t\t)
\t)
\t(property "Value" "Matrix8x8"
\t\t(at {body / 2} {body / 2 + 9} 0)
\t\t(layer "F.SilkS")
\t\t(uuid "{uid()}")
\t\t(effects
\t\t\t(font
\t\t\t\t(size 1 1)
\t\t\t\t(thickness 0.15)
\t\t\t)
\t\t)
\t)
\t(property "Datasheet" ""
\t\t(at 0 0 0)
\t\t(layer "F.Fab")
\t\t(hide yes)
\t\t(uuid "{uid()}")
\t\t(effects
\t\t\t(font
\t\t\t\t(size 1.27 1.27)
\t\t\t)
\t\t)
\t)
\t(property "Description" ""
\t\t(at 0 0 0)
\t\t(layer "F.Fab")
\t\t(hide yes)
\t\t(uuid "{uid()}")
\t\t(effects
\t\t\t(font
\t\t\t\t(size 1.27 1.27)
\t\t\t)
\t\t)
\t)
\t(attr through_hole)
\t(duplicate_pad_numbers_are_jumpers no)
\t(fp_rect
\t\t(start 0 0)
\t\t(end {body} {body})
\t\t(stroke
\t\t\t(width 0.1)
\t\t\t(type default)
\t\t)
\t\t(fill no)
\t\t(layer "F.SilkS")
\t\t(uuid "{uid()}")
\t)
\t(fp_arc
\t\t(start 0 15)
\t\t(mid 1 16)
\t\t(end 0 17)
\t\t(stroke
\t\t\t(width 0.1)
\t\t\t(type default)
\t\t)
\t\t(layer "F.SilkS")
\t\t(uuid "{uid()}")
\t)
\t(fp_rect
\t\t(start 0 0)
\t\t(end {body} {body})
\t\t(stroke
\t\t\t(width 0.05)
\t\t\t(type default)
\t\t)
\t\t(fill no)
\t\t(layer "F.CrtYd")
\t\t(uuid "{uid()}")
\t)
{chr(10).join(out_pads)}
{chr(10).join(out_lines)}
{chr(10).join(models)}
{matrix_body_model_block()}
\t(embedded_fonts no)
)
"""
    OUT.write_text(content, encoding="utf-8")
    print(f"Wrote {OUT} ({OUT.stat().st_size} bytes) from {SPEC.name}")


if __name__ == "__main__":
    main()
