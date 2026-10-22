#!/usr/bin/env python3
"""SKiDL netlist for the Maki Nita console.

Connections follow sim/board.c. J1 is the cart socket. The AVR128DB28
lives on the cartridge, so this board has no CPU. U2 to U5 are the
MAX7219 chain, left to right, top to bottom. DS1 to DS4 are the matrices
those drivers scan. The sim keeps the picture inside each driver. Here each
segment and digit pin lands on one matrix pad.

The matrix package pinout is a placeholder. Pad 1 is the marked end. Pads
1-8 step away from that mark at the footprint pitch. Pad 16 sits opposite
pad 1. Until a datasheet confirms the part, pads 1-8 are columns 0-7 and
pads 16-9 are rows 0-7, so column 0 and row 0 are both at the marked end.
Column 0 is segment DP. Row 0 is DIG0. That is the same left and top rule
as the sim.

Run from the repo:

    python3 kicad/skidl/maki_nita.py
"""

from __future__ import annotations

import builtins
import os
import sys
from pathlib import Path

_SYMBOLS = "/usr/share/kicad/symbols"
for _key in (
    "KICAD10_SYMBOL_DIR",
    "KICAD9_SYMBOL_DIR",
    "KICAD8_SYMBOL_DIR",
    "KICAD7_SYMBOL_DIR",
    "KICAD6_SYMBOL_DIR",
):
    os.environ.setdefault(_key, _SYMBOLS)

from skidl import KICAD10, Net, Part, Pin, SKIDL, generate_netlist
from skidl.pin import pin_types

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cart_edge import contacts, socket_pad, socket_pins
from net_classes import annotate_netlist_classes, apply_netclasses, patch_kicad_pro

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
NET_PATH = HERE / "maki-nita.net"
PRO_PATH = REPO / "kicad" / "main-pcb" / "maki-nita.kicad_pro"

SOCKET_FP = "MakiNita:EDAC_345_MoboSocket_2x8_2.54x5.08mm"
HDR_FP = "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical"
DRV_FP = "Package_DIP:DIP-24_W7.62mm"
MATRIX_FP = "MakiNita:Matrix8x8_2x8_P2.52mm_R25mm"

# Pin 1 is the square pad. Odd pins are one column, even pins the other.
HDR_PINS = (
    (1, "GND"),
    (2, "VCC"),
    (3, "UP"),
    (4, "DOWN"),
    (5, "LEFT"),
    (6, "RIGHT"),
    (7, "BUTTON"),
    (8, "GND2"),
)

# Column 0 is the left pixel, bit 7, segment DP. Same order as docs/hardware.md.
SEG_PINS = (22, 14, 16, 20, 23, 21, 15, 17)  # DP A B C D E F G
# Row 0 is the top of the block, DIG0.
DIG_PINS = (2, 11, 6, 7, 3, 10, 5, 8)  # DIG0 .. DIG7

# Placeholder pad numbers. See the module docstring.
COL_PADS = (1, 2, 3, 4, 5, 6, 7, 8)
ROW_PADS = (16, 15, 14, 13, 12, 11, 10, 9)

DRV_PINS = (
    (1, "DIN"),
    (2, "DIG0"),
    (3, "DIG4"),
    (4, "GND"),
    (5, "DIG6"),
    (6, "DIG2"),
    (7, "DIG3"),
    (8, "DIG7"),
    (9, "GND2"),
    (10, "DIG5"),
    (11, "DIG1"),
    (12, "LOAD"),
    (13, "CLK"),
    (14, "SEGA"),
    (15, "SEGF"),
    (16, "SEGB"),
    (17, "SEGG"),
    (18, "ISET"),
    (19, "V+"),
    (20, "SEGC"),
    (21, "SEGE"),
    (22, "SEGDP"),
    (23, "SEGD"),
    (24, "DOUT"),
)

POWER_NAMES = {"VDD", "VDDIO2", "AVDD", "V+", "GND", "GND2"}


def _pin(num: int, name: str) -> Pin:
    func = pin_types.PWRIN if name in POWER_NAMES else pin_types.PASSIVE
    return Pin(num=num, name=name, func=func)


def _part(mpn: str, ref: str, footprint: str, pins, value: str) -> Part:
    part = Part(
        tool=SKIDL,
        name=mpn,
        ref=ref,
        tag=ref,
        footprint=footprint,
        pins=[_pin(num, name) for num, name in pins],
        value=value,
    )
    return part


def _tie(part: Part, num: int, net: Net) -> None:
    part[num] += net


def _stub(part: Part, num: int) -> None:
    _tie(part, num, Net(f"NC_{part.ref}_{num}"))


def build() -> None:
    vcc = Net("+5V")
    gnd = Net("GND")
    din = Net("DIN")
    clk = Net("CLK")
    load = Net("LOAD")
    reset = Net("RESET")
    updi = Net("UPDI")
    switches = {
        "UP": Net("UP"),
        "DOWN": Net("DOWN"),
        "LEFT": Net("LEFT"),
        "RIGHT": Net("RIGHT"),
        "BUTTON": Net("BUTTON"),
    }

    named = {
        "DIN": din,
        "CLK": clk,
        "LOAD": load,
        "RESET": reset,
        "UPDI": updi,
        **switches,
    }
    socket = _part(
        "EDAC-345-016-520-212",
        "J1",
        SOCKET_FP,
        socket_pins(),
        "EDAC 345-016-520-212",
    )
    for number, side, net_name, _avr in contacts():
        pad = socket_pad(number, side)
        if net_name == "GND":
            _tie(socket, pad, gnd)
        elif net_name == "+5V":
            _tie(socket, pad, vcc)
        elif net_name is None:
            _stub(socket, pad)
        elif net_name in named:
            _tie(socket, pad, named[net_name])
        else:
            _tie(socket, pad, Net(net_name))

    header = _part("PinHeader_2x04", "J2", HDR_FP, HDR_PINS, "PinHeader 2x04")
    _tie(header, 1, gnd)
    _tie(header, 8, gnd)
    _tie(header, 2, vcc)
    _tie(header, 3, switches["UP"])
    _tie(header, 4, switches["DOWN"])
    _tie(header, 5, switches["LEFT"])
    _tie(header, 6, switches["RIGHT"])
    _tie(header, 7, switches["BUTTON"])

    drivers = []
    matrices = []
    for index in range(4):
        row = index // 2
        ref = f"U{index + 2}"
        drv = _part("MAX7219", ref, DRV_FP, DRV_PINS, "MAX7219")
        matrix_pins = tuple((pad, f"C{col}") for col, pad in enumerate(COL_PADS))
        matrix_pins += tuple((pad, f"R{row_i}") for row_i, pad in enumerate(ROW_PADS))
        matrix = _part(
            "Matrix8x8",
            f"DS{index + 1}",
            MATRIX_FP,
            matrix_pins,
            "Matrix8x8",
        )
        _tie(drv, 19, vcc)
        _tie(drv, 4, gnd)
        _tie(drv, 9, gnd)
        _tie(drv, 13, clk)
        _tie(drv, 12, load)
        _tie(drv, 18, Net(f"{ref}_ISET"))
        if index == 0:
            _tie(drv, 1, din)
        klass = f"ROW{row + 1}"
        for col, (seg_pin, pad) in enumerate(zip(SEG_PINS, COL_PADS)):
            net = Net(f"{klass}_{matrix.ref}_C{col}")
            _tie(drv, seg_pin, net)
            _tie(matrix, pad, net)
        for block_row, (dig_pin, pad) in enumerate(zip(DIG_PINS, ROW_PADS)):
            net = Net(f"{klass}_{matrix.ref}_R{block_row}")
            _tie(drv, dig_pin, net)
            _tie(matrix, pad, net)
        drivers.append(drv)
        matrices.append(matrix)

    for index in range(3):
        link = Net(f"DOUT_{drivers[index].ref}")
        _tie(drivers[index], 24, link)
        _tie(drivers[index + 1], 1, link)
    _tie(drivers[-1], 24, Net("DOUT"))


def _check(text: str) -> None:
    import re

    from net_classes import class_for_name

    def nodes(name: str) -> set[tuple[str, str]]:
        match = re.search(
            rf'\(name "{re.escape(name)}"\) \(class "([^"]+)"\)',
            text,
        )
        if match is None:
            raise SystemExit(f"missing net {name}")
        if match.group(1) != class_for_name(name):
            raise SystemExit(f"{name} class is {match.group(1)}")
        start = match.end()
        nxt = re.search(r"\n\s*\(net\b", text[start:])
        chunk = text[start : start + (nxt.start() if nxt else len(text))]
        return set(re.findall(r'\(ref "([^"]+)"\)\s*\(pin "([^"]+)"\)', chunk))

    if '(value "EDAC 345-016-520-212")' not in text:
        raise SystemExit("cart socket is not the 16-contact EDAC")
    if text.count(f'(footprint "{SOCKET_FP}")') != 1:
        raise SystemExit("cart socket footprint missing")
    if text.count(f'(footprint "{HDR_FP}")') != 1:
        raise SystemExit("control header footprint missing")
    if text.count('(footprint "Package_DIP:DIP-24_W7.62mm")') != 4:
        raise SystemExit("expected 4 MAX7219 footprints")
    if text.count(f'(footprint "{MATRIX_FP}")') != 4:
        raise SystemExit("expected 4 matrix footprints")
    vcc = nodes("+5V")
    gnd = nodes("GND")
    if ("J1", "3") not in vcc or ("J1", "4") not in vcc or ("J1", "16") not in vcc:
        raise SystemExit("+5V does not reach three cart socket contacts")
    if ("J2", "2") not in vcc:
        raise SystemExit("+5V does not reach the header")
    drivers_on_vcc = {ref for ref, pin in vcc if pin == "19"}
    if drivers_on_vcc != {f"U{n}" for n in range(2, 6)}:
        raise SystemExit(f"+5V drivers {sorted(drivers_on_vcc)}")
    for pad in ("1", "2", "15"):
        if ("J1", pad) not in gnd:
            raise SystemExit(f"GND does not reach cart socket pad {pad}")
    if ("J2", "1") not in gnd or ("J2", "8") not in gnd:
        raise SystemExit("GND does not reach both header ground pins")
    if ("U2", "4") not in gnd or ("U2", "9") not in gnd:
        raise SystemExit("GND does not reach both MAX7219 ground pins")
    din = nodes("DIN")
    if din != {("J1", "8"), ("U2", "1")}:
        raise SystemExit(f"DIN nodes {din}")
    clk = nodes("CLK")
    if ("J1", "10") not in clk or ("U2", "13") not in clk:
        raise SystemExit(f"CLK nodes {clk}")
    load = nodes("LOAD")
    if ("J1", "6") not in load or ("U2", "12") not in load:
        raise SystemExit(f"LOAD nodes {load}")
    up = nodes("UP")
    if up != {("J1", "5"), ("J2", "3")}:
        raise SystemExit(f"UP nodes {up}")
    button = nodes("BUTTON")
    if button != {("J1", "13"), ("J2", "7")}:
        raise SystemExit(f"BUTTON nodes {button}")
    link = nodes("DOUT_U2")
    if link != {("U2", "24"), ("U3", "1")}:
        raise SystemExit(f"DOUT_U2 nodes {link}")
    col0 = nodes("ROW1_DS1_C0")
    if col0 != {("U2", "22"), ("DS1", "1")}:
        raise SystemExit(f"ROW1_DS1_C0 nodes {col0}")
    row0 = nodes("ROW1_DS1_R0")
    if row0 != {("U2", "2"), ("DS1", "16")}:
        raise SystemExit(f"ROW1_DS1_R0 nodes {row0}")
    last = nodes("ROW2_DS4_C7")
    if last != {("U5", "17"), ("DS4", "8")}:
        raise SystemExit(f"ROW2_DS4_C7 nodes {last}")


def _clean_strays() -> None:
    for folder in (REPO, HERE, Path.cwd()):
        for name in ("skidl_REPL.log", "skidl_REPL.erc", "maki_nita.log", "maki_nita.erc"):
            path = folder / name
            if path.is_file():
                path.unlink()


def main() -> None:
    build()

    nets = []
    for part in builtins.default_circuit.parts:
        for pin in part.pins:
            if pin.net is not None:
                nets.append(pin.net)
    apply_netclasses(set(nets))
    generate_netlist(file_=str(NET_PATH), do_backup=False, tool=KICAD10)
    text = annotate_netlist_classes(NET_PATH.read_text(encoding="utf-8"))
    NET_PATH.write_text(text, encoding="utf-8")
    _check(text)
    if PRO_PATH.is_file():
        old = PRO_PATH.read_text(encoding="utf-8")
        new = patch_kicad_pro(old)
        if new != old:
            PRO_PATH.write_text(new, encoding="utf-8")
    print(f"wrote {NET_PATH}")
    _clean_strays()


if __name__ == "__main__":
    main()
