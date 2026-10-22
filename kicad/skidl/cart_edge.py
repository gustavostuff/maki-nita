"""Shared 36-contact cart edge.

The console uses an EDAC 395-036-520-201 socket. The cartridge uses the
matching gold fingers. Side A and side B are the two faces of the edge.
On the socket, side A is the odd pads and side B is the even pads.
On the fingers, side A is pads 1 to 18 and side B is pads 19 to 36.

A contact is (number 1..18, side, net, avr package pin or None).
AVR pins 6, 14, and 20 are +5V on the cartridge. Pins 15 and 21 are GND.
"""

from __future__ import annotations

# (contact, A net, A avr pin, B net, B avr pin)
_ROWS = (
    (1, "GND", None, "GND", None),
    (2, "+5V", None, "+5V", None),
    (3, "LOAD", 1, "PC0", 2),
    (4, "PC1", 3, "PC2", 4),
    (5, "PC3", 5, "UP", 7),
    (6, "DOWN", 8, "LEFT", 9),
    (7, "RIGHT", 10, "BUTTON", 11),
    (8, "PD6", 12, "PD7", 13),
    (9, "PF0", 16, "PF1", 17),
    (10, "RESET", 18, "UPDI", 19),
    (11, "PA0", 22, "PA1", 23),
    (12, "PA2", 24, "PA3", 25),
    (13, "DIN", 26, "PA5", 27),
    (14, "CLK", 28, None, None),
    (15, None, None, None, None),
    (16, None, None, None, None),
    (17, None, None, None, None),
    (18, "GND", None, "GND", None),
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


def finger_pad(number: int, side: str) -> int:
    if side == "A":
        return number
    return number + 18


def socket_pins():
    pins = []
    for number, side, _net, _avr in contacts():
        pins.append((socket_pad(number, side), f"{side}{number}"))
    return pins


def finger_pins():
    pins = []
    for number, side, _net, _avr in contacts():
        pins.append((finger_pad(number, side), f"{side}{number}"))
    return pins
