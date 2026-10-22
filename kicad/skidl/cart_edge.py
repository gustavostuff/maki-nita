"""Shared 16-contact cart edge.

The console socket is an EDAC 345-016-520-212, eight contacts a side.
Side A is the odd pads. Side B is the even pads. Pad 1 is contact A1.

The cartridge can be inserted either way around. Turning it around swaps
the two faces and reverses the ends, so side B carries side A in reverse.
A contact is (number 1..8, side, net, avr package pin or None).

AVR pins 6, 14, and 20 are +5V. Pins 15 and 21 are GND.
RIGHT, BUTTON, RESET, and UPDI are not on this edge. Eight contacts
give eight nets once the back side repeats the front side in reverse.
"""

from __future__ import annotations

# (contact, A net, A avr pin, B net, B avr pin)
# B row n is the same net as A row (9-n).
_ROWS = (
    (1, "GND", None, "CLK", 28),
    (2, "+5V", None, "DIN", 26),
    (3, "LOAD", 1, "LEFT", 9),
    (4, "UP", 7, "DOWN", 8),
    (5, "DOWN", 8, "UP", 7),
    (6, "LEFT", 9, "LOAD", 1),
    (7, "DIN", 26, "+5V", None),
    (8, "CLK", 28, "GND", None),
)


def contacts():
    rows = []
    for number, a_net, a_avr, b_net, b_avr in _ROWS:
        rows.append((number, "A", a_net, a_avr))
        rows.append((number, "B", b_net, b_avr))
    return rows


def socket_pad(number: int, side: str) -> int:
    if side == "A":
        return 2 * number - 1
    return 2 * number


def socket_pins():
    pins = []
    for number, side, _net, _avr in contacts():
        pins.append((socket_pad(number, side), f"{side}{number}"))
    return pins


def finger_pins():
    pins = []
    for number, side, _net, _avr in contacts():
        name = f"{side}{number}"
        pins.append((name, name))
    return pins
