"""Shared 16-contact cart edge.

The console socket is an EDAC 345-016-520-212, eight contacts a side.
Side A is the odd pads and the controller face. Side B is the even pads
and carries the display and programming signals. Pad 1 is contact A1.

Three contacts are GND and three are +5V. The ends of the edge are power.
A contact is (number 1..8, side, net, avr package pin or None).
AVR pins 6, 14, and 20 are +5V. Pins 15 and 21 are GND.
"""

from __future__ import annotations

# (contact, A net, A avr pin, B net, B avr pin)
_ROWS = (
    (1, "GND", None, "GND", None),
    (2, "+5V", None, "+5V", None),
    (3, "UP", 7, "LOAD", 1),
    (4, "DOWN", 8, "DIN", 26),
    (5, "LEFT", 9, "CLK", 28),
    (6, "RIGHT", 10, "RESET", 18),
    (7, "BUTTON", 11, "UPDI", 19),
    (8, "GND", None, "+5V", None),
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
