"""Shared 16-contact cart edge.

The console uses an EDAC 345-016-520-212 socket, eight contacts a side.
Side A is the odd pads and side B is the even pads. Pad 1 is contact A1.

A contact is (number 1..8, side, net, avr package pin or None).
AVR pins 6, 14, and 20 are +5V. Pins 15 and 21 are GND.
The spare port pins are not on this connector.
"""

from __future__ import annotations

# (contact, A net, A avr pin, B net, B avr pin)
_ROWS = (
    (1, "GND", None, "GND", None),
    (2, "+5V", None, "+5V", None),
    (3, "LOAD", 1, "UP", 7),
    (4, "DOWN", 8, "LEFT", 9),
    (5, "RIGHT", 10, "BUTTON", 11),
    (6, "RESET", 18, "UPDI", 19),
    (7, "DIN", 26, "CLK", 28),
    (8, "GND", None, "GND", None),
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
