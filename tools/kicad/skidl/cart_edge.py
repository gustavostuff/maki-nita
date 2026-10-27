"""Shared 20-contact cart edge.

The console socket is an EDAC 395-020-520-202, ten contacts a side.
Side A sits at the bottom of the connector on the main PCB, carrying
player inputs. Side B sits at the top of the connector on the main PCB,
carrying power, audio events, reset, programming, and display signals.
"""

from __future__ import annotations

# (contact, A net, A avr pin, B net, B avr pin)
_ROWS = (
    (1, "P1_LEFT", None, "+5V", None),
    (2, "P1_RIGHT", None, "GND", None),
    (3, "P1_UP", None, "EVENT", 22),
    (4, "P1_DOWN", None, "RESET", 18),
    (5, "P1_BTN", None, "UPDI", 19),
    (6, "P2_LEFT", None, "GND", None),
    (7, "P2_RIGHT", None, "GND", None),
    (8, "P2_UP", None, "LOAD", 1),
    (9, "P2_DOWN", None, "DIN", 26),
    (10, "P2_BTN", None, "CLK", 28),
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
