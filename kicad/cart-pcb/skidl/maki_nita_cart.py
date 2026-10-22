#!/usr/bin/env python3
"""SKiDL netlist for a Maki Nita cartridge.

The cartridge holds one AVR128DB28. The edge footprint is not in this
netlist yet. VDD, AVDD, and VDDIO2 tie to +5V. Both ground pins tie to GND.
"""

from __future__ import annotations

import os
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

HERE = Path(__file__).resolve().parent
NET_PATH = HERE / "maki-nita-cart.net"

AVR_FP = "Package_DIP:DIP-28_W7.62mm"

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

    if "BUS_PCIexpress" in text:
        raise SystemExit("PCIe edge is still in the cartridge netlist")
    if text.count(f'(footprint "{AVR_FP}")') != 1:
        raise SystemExit("AVR footprint missing")
    vcc = nodes("+5V")
    if ("U1", "20") not in vcc or ("U1", "14") not in vcc or ("U1", "6") not in vcc:
        raise SystemExit("+5V does not reach the AVR rails")
    gnd = nodes("GND")
    if ("U1", "15") not in gnd or ("U1", "21") not in gnd:
        raise SystemExit("GND does not reach both AVR ground pins")


def main() -> None:
    build()
    generate_netlist(file_=str(NET_PATH), do_backup=False, tool=KICAD10)
    text = NET_PATH.read_text(encoding="utf-8")
    _check(text)
    print(f"wrote {NET_PATH}")


if __name__ == "__main__":
    main()
