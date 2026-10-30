#!/usr/bin/env python3
"""SKiDL netlist for a MakiNeat cartridge.

The cartridge holds one AVR128DB28 and a double-sided 2x10 gold edge.
Side A is the controller face (Player 1 and Player 2). Side B carries
the display, EVENT, power, reset, and the programming pins. VDD, AVDD,
and VDDIO2 tie to +5V. Both ground pins tie to GND. PA0 drives EVENT.
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

sys.path.insert(0, str(Path(__file__).resolve().parents[4]))
from tools.kicad.shared.cart_edge import contacts, finger_pins

HERE = Path(__file__).resolve().parent
NET_PATH = HERE / "makineat-cart.net"

AVR_FP = "Package_DIP:DIP-28_W7.62mm"
FINGER_FP = "MakiNeat:Cart_Edge_2x10_P2.54mm"
C_FP = "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P2.50mm"
TWO_PINS = ((1, "1"), (2, "2"))

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


def _passive(ref: str, footprint: str, value: str, description: str = "") -> Part:
    return Part(
        tool=SKIDL,
        name="Passive",
        ref=ref,
        tag=ref,
        footprint=footprint,
        pins=[_pin(num, name) for num, name in TWO_PINS],
        value=value,
        description=description or value,
    )


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

    c1 = _passive("C1", C_FP, "100 nF", "100 nF VDD")
    c1[1] += vcc
    c1[2] += gnd

    c2 = _passive("C2", C_FP, "100 nF", "100 nF AVDD")
    c2[1] += vcc
    c2[2] += gnd

    c3 = _passive("C3", C_FP, "100 nF", "100 nF VDDIO2")
    c3[1] += vcc
    c3[2] += gnd

    edge = Part(
        tool=SKIDL,
        name="Cart_Edge_2x10",
        ref="J1",
        tag="J1",
        footprint=FINGER_FP,
        pins=[_pin(num, name) for num, name in finger_pins()],
        value="Cart edge 2x10",
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

    for num, _name in AVR_PINS:
        if cpu[num].net is None:
            cpu[num] += Net(f"NC_U1_{num}")


CLASS_5V = "5V"
CLASS_SIDE_A = "SIDE_A"
CLASS_SIDE_B = "SIDE_B"

SIDE_A_NETS = frozenset(
    (
        "P1_UP",
        "P1_DOWN",
        "P1_LEFT",
        "P1_RIGHT",
        "P1_BTN",
        "P2_UP",
        "P2_DOWN",
        "P2_LEFT",
        "P2_RIGHT",
        "P2_BTN",
    )
)
SIDE_B_NETS = frozenset(("CLK", "DIN", "EVENT", "LOAD", "RESET", "UPDI"))


def class_for_name(name: str) -> str:
    if name == "+5V":
        return CLASS_5V
    if name in SIDE_A_NETS:
        return CLASS_SIDE_A
    if name in SIDE_B_NETS:
        return CLASS_SIDE_B
    return "Default"


def annotate_netlist_classes(text: str) -> str:
    import re

    class_re = re.compile(
        r'(\(code\s+\d+\)\s*)\(name "([^"]+)"\)(?:\s*\(class "[^"]*"\))?'
    )

    def repl(match: re.Match) -> str:
        name = match.group(2)
        return f'{match.group(1)}(name "{name}") (class "{class_for_name(name)}")'

    return class_re.sub(repl, text)


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
    if text.count(f'(footprint "{C_FP}")') != 3:
        raise SystemExit("Capacitor footprint count mismatch")
    vcc = nodes("+5V")
    if ("U1", "20") not in vcc or ("U1", "14") not in vcc or ("U1", "6") not in vcc:
        raise SystemExit("+5V does not reach the AVR rails")
    if ("J1", "B10") not in vcc:
        raise SystemExit("+5V does not reach edge contact B10")
    for ref in ("C1", "C2", "C3"):
        if (ref, "1") not in vcc:
            raise SystemExit(f"{ref} pin 1 not on +5V")
    gnd = nodes("GND")
    if ("U1", "15") not in gnd or ("U1", "21") not in gnd:
        raise SystemExit("GND does not reach both AVR ground pins")
    for pad in ("B4", "B5", "B9"):
        if ("J1", pad) not in gnd:
            raise SystemExit(f"GND does not reach edge contact {pad}")
    for ref in ("C1", "C2", "C3"):
        if (ref, "2") not in gnd:
            raise SystemExit(f"{ref} pin 2 not on GND")
    for number, side, net_name, avr in contacts():
        if net_name in ("+5V", "GND"):
            continue
        expected = {("J1", f"{side}{number}")}
        if avr is not None:
            expected.add(("U1", str(avr)))
        actual = nodes(net_name)
        if actual != expected:
            raise SystemExit(f"{net_name} nodes mismatch: actual {actual} != expected {expected}")
    for num in (12, 13, 16, 17, 23, 24, 25):
        nc = nodes(f"NC_U1_{num}")
        if nc != {("U1", str(num))}:
            raise SystemExit(f"NC_U1_{num} mismatch: {nc}")
    for name in SIDE_A_NETS:
        if f'(name "{name}") (class "SIDE_A")' not in text:
            raise SystemExit(f"missing SIDE_A class on {name}")
    for name in SIDE_B_NETS:
        if f'(name "{name}") (class "SIDE_B")' not in text:
            raise SystemExit(f"missing SIDE_B class on {name}")
    if '(name "+5V") (class "5V")' not in text:
        raise SystemExit("missing 5V class on +5V")


def main() -> None:
    build()
    generate_netlist(file_=str(NET_PATH), do_backup=False, tool=KICAD10)
    text = NET_PATH.read_text(encoding="utf-8")
    text = annotate_netlist_classes(text)
    NET_PATH.write_text(text, encoding="utf-8")
    _check(text)
    print(f"wrote {NET_PATH}")


if __name__ == "__main__":
    main()
