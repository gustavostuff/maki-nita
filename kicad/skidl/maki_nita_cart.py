#!/usr/bin/env python3
"""SKiDL netlist for a Maki Nita cartridge.

The cartridge holds one AVR128DB28. J1 is the gold edge that mates with
the console socket. Power pins of the AVR tie to the edge +5V and GND
contacts. Every other package pin lands on the contact listed in cart_edge.
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

from skidl import KICAD10, Net, generate_netlist

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cart_edge import contacts, finger_pad, finger_pins
from maki_nita import _part, _tie

HERE = Path(__file__).resolve().parent
NET_PATH = HERE / "maki-nita-cart.net"

AVR_FP = "Package_DIP:DIP-28_W7.62mm"
FINGER_FP = "MakiNita:Cart_Edge_2x18_P2.54mm"

AVR_PINS = (
    (1, "PA7"),
    (2, "PC0"),
    (3, "PC1"),
    (4, "PC2"),
    (5, "PC3"),
    (6, "VDDIO2"),
    (7, "PD1"),
    (8, "PD2"),
    (9, "PD3"),
    (10, "PD4"),
    (11, "PD5"),
    (12, "PD6"),
    (13, "PD7"),
    (14, "AVDD"),
    (15, "GND"),
    (16, "PF0"),
    (17, "PF1"),
    (18, "PF6"),
    (19, "UPDI"),
    (20, "VDD"),
    (21, "GND2"),
    (22, "PA0"),
    (23, "PA1"),
    (24, "PA2"),
    (25, "PA3"),
    (26, "PA4"),
    (27, "PA5"),
    (28, "PA6"),
)


def build() -> None:
    vcc = Net("+5V")
    gnd = Net("GND")
    cpu = _part("AVR128DB28-I/SP", "U1", AVR_FP, AVR_PINS, "AVR128DB28-I/SP")
    edge = _part("Cart_Edge_2x18", "J1", FINGER_FP, finger_pins(), "Cart edge 2x18")
    _tie(cpu, 20, vcc)
    _tie(cpu, 14, vcc)
    _tie(cpu, 6, vcc)
    _tie(cpu, 15, gnd)
    _tie(cpu, 21, gnd)

    nets = {"+5V": vcc, "GND": gnd}
    for number, side, net_name, avr in contacts():
        pad = finger_pad(number, side)
        if net_name is None:
            _tie(edge, pad, Net(f"NC_J1_{side}{number}"))
            continue
        if net_name not in nets:
            nets[net_name] = Net(net_name)
        _tie(edge, pad, nets[net_name])
        if avr is not None:
            _tie(cpu, avr, nets[net_name])


def _check(text: str) -> None:
    import re

    def nodes(name: str) -> set[tuple[str, str]]:
        match = re.search(rf'\(name "{re.escape(name)}"\)', text)
        if match is None:
            raise SystemExit(f"missing net {name}")
        start = match.end()
        nxt = re.search(r"\n\s*\(net\b", text[start:])
        chunk = text[start : start + (nxt.start() if nxt else len(text))]
        return set(re.findall(r'\(ref "([^"]+)"\)\s*\(pin "([^"]+)"\)', chunk))

    if text.count(f'(footprint "{AVR_FP}")') != 1:
        raise SystemExit("AVR footprint missing")
    if text.count(f'(footprint "{FINGER_FP}")') != 1:
        raise SystemExit("cart edge footprint missing")
    vcc = nodes("+5V")
    if ("U1", "20") not in vcc or ("U1", "14") not in vcc or ("U1", "6") not in vcc:
        raise SystemExit("+5V does not reach the AVR rails")
    if ("J1", "2") not in vcc or ("J1", "20") not in vcc:
        raise SystemExit("+5V does not reach both edge contacts")
    gnd = nodes("GND")
    if ("U1", "15") not in gnd or ("U1", "21") not in gnd:
        raise SystemExit("GND does not reach both AVR ground pins")
    din = nodes("DIN")
    if din != {("U1", "26"), ("J1", "13")}:
        raise SystemExit(f"DIN nodes {din}")
    clk = nodes("CLK")
    if clk != {("U1", "28"), ("J1", "14")}:
        raise SystemExit(f"CLK nodes {clk}")
    load = nodes("LOAD")
    if load != {("U1", "1"), ("J1", "3")}:
        raise SystemExit(f"LOAD nodes {load}")


def main() -> None:
    build()
    generate_netlist(file_=str(NET_PATH), do_backup=False, tool=KICAD10)
    text = NET_PATH.read_text(encoding="utf-8")
    _check(text)
    print(f"wrote {NET_PATH}")


if __name__ == "__main__":
    main()
