#!/usr/bin/env python3
"""SKiDL netlist for a Maki Nita cartridge.

The cartridge holds one AVR128DB28 and a double-sided 2x8 gold edge.
Side A is the controller face. Side B carries the display, EVENT, and
the programming pins. VDD, AVDD, and VDDIO2 tie to +5V. Both ground pins
tie to GND. PA0 drives EVENT.
"""

from __future__ import annotations

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

from skidl import KICAD10, Net, Pin, Part, SKIDL, generate_netlist
from skidl.pin import pin_types

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "skidl"))
from cart_edge import contacts, finger_pins

HERE = Path(__file__).resolve().parent
NET_PATH = HERE / "maki-nita-cart.net"

AVR_FP = "Package_DIP:DIP-28_W7.62mm"
FINGER_FP = "MakiNita:Cart_Edge_2x8_P2.54mm"

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

POWER_NAMES = {"VDD", "VDDIO2", "AVDD", "GND", "GND2"}


def _pin(num, name: str) -> Pin:
    func = pin_types.PWRIN if name in POWER_NAMES else pin_types.PASSIVE
    return Pin(num=num, name=name, func=func)


def build() -> None:
    vcc = Net("+5V")
    gnd = Net("GND")
    cpu = Part(
        tool=SKIDL,
        name="AVR128DB28-I/SP",
        ref="U1",
        tag="U1",
        footprint=AVR_FP,
        pins=[_pin(num, name) for num, name in AVR_PINS],
        value="AVR128DB28-I/SP",
    )
    for num in (20, 14, 6):
        cpu[num] += vcc
    for num in (15, 21):
        cpu[num] += gnd

    edge = Part(
        tool=SKIDL,
        name="Cart_Edge_2x8",
        ref="J1",
        tag="J1",
        footprint=FINGER_FP,
        pins=[_pin(num, name) for num, name in finger_pins()],
        value="Cart edge 2x8",
    )
    nets = {"+5V": vcc, "GND": gnd}
    tied_avr = set()
    for number, side, net_name, avr in contacts():
        if net_name not in nets:
            nets[net_name] = Net(net_name)
        edge[f"{side}{number}"] += nets[net_name]
        if avr is not None and avr not in tied_avr:
            cpu[avr] += nets[net_name]
            tied_avr.add(avr)


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
        raise SystemExit("gold finger footprint missing")
    vcc = nodes("+5V")
    if ("U1", "20") not in vcc or ("U1", "14") not in vcc or ("U1", "6") not in vcc:
        raise SystemExit("+5V does not reach the AVR rails")
    if ("J1", "A2") not in vcc or ("J1", "B2") not in vcc or ("J1", "B8") not in vcc:
        raise SystemExit("+5V does not reach three edge contacts")
    gnd = nodes("GND")
    if ("U1", "15") not in gnd or ("U1", "21") not in gnd:
        raise SystemExit("GND does not reach both AVR ground pins")
    if ("J1", "A1") not in gnd or ("J1", "A8") not in gnd:
        raise SystemExit("GND does not reach both end contacts")
    if ("J1", "B1") in gnd:
        raise SystemExit("B1 is EVENT")
    event = nodes("EVENT")
    if event != {("U1", "22"), ("J1", "B1")}:
        raise SystemExit(f"EVENT nodes {event}")
    din = nodes("DIN")
    if din != {("U1", "26"), ("J1", "B4")}:
        raise SystemExit(f"DIN nodes {din}")
    clk = nodes("CLK")
    if clk != {("U1", "28"), ("J1", "B5")}:
        raise SystemExit(f"CLK nodes {clk}")
    load = nodes("LOAD")
    if load != {("U1", "1"), ("J1", "B3")}:
        raise SystemExit(f"LOAD nodes {load}")
    button = nodes("BUTTON")
    if button != {("U1", "11"), ("J1", "A7")}:
        raise SystemExit(f"BUTTON nodes {button}")
    updi = nodes("UPDI")
    if updi != {("U1", "19"), ("J1", "B7")}:
        raise SystemExit(f"UPDI nodes {updi}")


def main() -> None:
    build()
    generate_netlist(file_=str(NET_PATH), do_backup=False, tool=KICAD10)
    text = NET_PATH.read_text(encoding="utf-8")
    _check(text)
    print(f"wrote {NET_PATH}")


if __name__ == "__main__":
    main()
