#!/usr/bin/env python3
"""SKiDL netlist for the Maki-Nita console.

J1 is the cart socket. The game AVR lives on the cartridge. U6 is the
audio AVR and U7 is the LM386. U2 to U5 are the MAX7219 chain, left to
right, top to bottom. DS1 to DS4 are the matrices those drivers scan.
The sim keeps the picture inside each driver. It models the audio AVR
on EVENT, and it models J4. Here each segment and digit pin lands on
one matrix pad.

The matrix package pinout is a placeholder. Pad 1 is the marked end. Pads
1-8 step away from that mark at the footprint pitch. Pad 16 sits opposite
pad 1. Until a datasheet confirms the part, pads 1-8 are columns 0-7 and
pads 16-9 are rows 0-7, so column 0 and row 0 are both at the marked end.
Column 0 is segment DP. Row 0 is DIG0. That is the same left and top rule
as the sim.

Run from the repo:

    python3 tools/kicad/skidl/maki_nita.py
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
PWR_FP = "Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical"
SPK_FP = "Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical"
UPDI_FP = "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical"
DRV_FP = "Package_DIP:DIP-24_W7.62mm"
AUDIO_FP = "Package_DIP:DIP-28_W7.62mm"
AMP_FP = "Package_DIP:DIP-8_W7.62mm"
MATRIX_FP = "MakiNita:Matrix8x8_2x8_P2.52mm_R25mm"
R_FP = "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm"
C_ELEC_FP = "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm"
C_DISC_FP = "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm"

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

# Pin 1 is the square pad. The row with pin 1 is power. The other row is reset.
PWR_PINS = (
    (1, "GND"),
    (2, "VCC"),
    (3, "RESET"),
    (4, "GND2"),
)

# Pin 1 is the square pad. Pins 1 and 4 are ground. Pins 2 and 3 are the speaker.
SPK_PINS = (
    (1, "GND"),
    (2, "SPK"),
    (3, "SPK2"),
    (4, "GND2"),
)

# Column 0 is the left pixel, bit 7, segment DP. Same order as docs/video.md.
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

# Same 28-pin map as the cartridge. PD6 is the DAC. PA1 listens to EVENT.
AUDIO_PINS = (
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

AMP_PINS = (
    (1, "GAIN"),
    (2, "INN"),
    (3, "INP"),
    (4, "GND"),
    (5, "VOUT"),
    (6, "VS"),
    (7, "BYPASS"),
    (8, "GAINB"),
)

TWO_PINS = ((1, "1"), (2, "2"))

POWER_NAMES = {"VDD", "VDDIO2", "AVDD", "V+", "VS", "GND", "GND2"}


def _pin(num: int, name: str) -> Pin:
    func = pin_types.PWRIN if name in POWER_NAMES else pin_types.PASSIVE
    return Pin(num=num, name=name, func=func)


def _part(mpn: str, ref: str, footprint: str, pins, value: str, description: str = "") -> Part:
    part = Part(
        tool=SKIDL,
        name=mpn,
        ref=ref,
        tag=ref,
        footprint=footprint,
        pins=[_pin(num, name) for num, name in pins],
        value=value,
        description=description or value,
    )
    return part


def _tie(part: Part, num: int, net: Net) -> None:
    part[num] += net


def _stub(part: Part, num: int) -> None:
    _tie(part, num, Net(f"NC_{part.ref}_{num}"))


def _passive(ref: str, footprint: str, value: str, description: str = "") -> Part:
    return _part("Passive", ref, footprint, TWO_PINS, value, description)


def _audio(vcc: Net, gnd: Net, event: Net, reset: Net) -> None:
    """Console synth. PD6 feeds the LM386 through the divider in the audio doc."""
    cpu = _part(
        "AVR128DB28-I/SP",
        "U6",
        AUDIO_FP,
        AUDIO_PINS,
        "Audio AVR",
        "AVR128DB28-I/SP",
    )
    for num in (20, 14, 6):
        _tie(cpu, num, vcc)
    for num in (15, 21):
        _tie(cpu, num, gnd)
    _tie(cpu, 18, reset)
    _tie(cpu, 23, event)

    updi = Net("AUDIO_UPDI")
    _tie(cpu, 19, updi)
    prog = _part(
        "PinHeader_1x03",
        "J5",
        UPDI_FP,
        ((1, "UPDI"), (2, "VCC"), (3, "GND")),
        "UPDI",
        "Audio programming",
    )
    _tie(prog, 1, updi)
    _tie(prog, 2, vcc)
    _tie(prog, 3, gnd)

    dac = Net("DAC")
    _tie(cpu, 12, dac)
    pull = _passive("R1", R_FP, "10k EVENT", "10 kohm EVENT pull-up")
    _tie(pull, 1, event)
    _tie(pull, 2, vcc)
    sink = _passive("R2", R_FP, "10k DAC", "10 kohm DAC load")
    _tie(sink, 1, dac)
    _tie(sink, 2, gnd)

    aud = Net("AUD")
    couple = _passive("C1", C_ELEC_FP, "1u DAC", "1 uF DAC coupling")
    _tie(couple, 1, dac)
    _tie(couple, 2, aud)
    audin = Net("AUDIN")
    series = _passive("R3", R_FP, "22k IN", "22 kohm into the LM386")
    _tie(series, 1, aud)
    _tie(series, 2, audin)
    shunt = _passive("R4", R_FP, "1k IN", "1 kohm LM386 input shunt")
    _tie(shunt, 1, audin)
    _tie(shunt, 2, gnd)

    amp = _part("LM386N-1", "U7", AMP_FP, AMP_PINS, "LM386", "LM386N-1 speaker amplifier")
    _tie(amp, 2, gnd)
    _tie(amp, 3, audin)
    _tie(amp, 4, gnd)
    _tie(amp, 6, vcc)
    bypass = Net("BYPASS")
    _tie(amp, 7, bypass)
    amp_out = Net("AMP")
    _tie(amp, 5, amp_out)

    spk = Net("SPK")
    output = _passive("C2", C_ELEC_FP, "220u SPK", "220 uF speaker coupling")
    _tie(output, 1, amp_out)
    _tie(output, 2, spk)
    speaker = _part(
        "PinHeader_2x02",
        "J4",
        SPK_FP,
        SPK_PINS,
        "Speaker",
        "Speaker header",
    )
    _tie(speaker, 1, gnd)
    _tie(speaker, 2, spk)
    _tie(speaker, 3, spk)
    _tie(speaker, 4, gnd)

    zobel = Net("ZOBEL")
    zobel_r = _passive("R5", R_FP, "10R ZOB", "10 ohm Zobel")
    _tie(zobel_r, 1, amp_out)
    _tie(zobel_r, 2, zobel)
    zobel_c = _passive("C3", C_DISC_FP, "47n ZOB", "47 nF Zobel")
    _tie(zobel_c, 1, zobel)
    _tie(zobel_c, 2, gnd)

    c4 = _passive("C4", C_DISC_FP, "U6 VDD", "100 nF on U6 pins 20 and 21")
    _tie(c4, 1, vcc)
    _tie(c4, 2, gnd)
    c6 = _passive("C6", C_DISC_FP, "U6 AVDD", "100 nF on U6 pins 14 and 15")
    _tie(c6, 1, vcc)
    _tie(c6, 2, gnd)
    c7 = _passive("C7", C_DISC_FP, "U6 IO", "100 nF on U6 pins 6 and 21")
    _tie(c7, 1, vcc)
    _tie(c7, 2, gnd)
    c8 = _passive("C8", C_DISC_FP, "U7 VS", "100 nF on U7 pins 6 and 4")
    _tie(c8, 1, vcc)
    _tie(c8, 2, gnd)
    bulk = _passive("C5", C_ELEC_FP, "U7 VS", "100 uF on U7 pins 6 and 4")
    _tie(bulk, 1, vcc)
    _tie(bulk, 2, gnd)
    c9 = _passive("C9", C_ELEC_FP, "U7 BYP", "10 uF on U7 pins 7 and 4")
    _tie(c9, 1, bypass)
    _tie(c9, 2, gnd)


def build() -> None:
    # Two global power nets. Every rail pin and every decoupling cap lands here.
    vcc = Net("+5V")
    gnd = Net("GND")
    din = Net("DIN")
    clk = Net("CLK")
    load = Net("LOAD")
    reset = Net("RESET")
    updi = Net("UPDI")
    event = Net("EVENT")
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
        "EVENT": event,
        **switches,
    }
    socket = _part(
        "EDAC-345-016-520-212",
        "J1",
        SOCKET_FP,
        socket_pins(),
        "Cart",
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

    header = _part("PinHeader_2x04", "J2", HDR_FP, HDR_PINS, "Controls", "Stick and button")
    _tie(header, 1, gnd)
    _tie(header, 8, gnd)
    _tie(header, 2, vcc)
    _tie(header, 3, switches["UP"])
    _tie(header, 4, switches["DOWN"])
    _tie(header, 5, switches["LEFT"])
    _tie(header, 6, switches["RIGHT"])
    _tie(header, 7, switches["BUTTON"])

    power = _part("PinHeader_2x02", "J3", PWR_FP, PWR_PINS, "Power", "5 V and reset")
    _tie(power, 1, gnd)
    _tie(power, 2, vcc)
    _tie(power, 3, reset)
    _tie(power, 4, gnd)
    _audio(vcc, gnd, event, reset)

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

    if '(value "Cart")' not in text or '(description "EDAC 345-016-520-212")' not in text:
        raise SystemExit("cart socket is not the 16-contact EDAC")
    if text.count(f'(footprint "{SOCKET_FP}")') != 1:
        raise SystemExit("cart socket footprint missing")
    if text.count(f'(footprint "{HDR_FP}")') != 1:
        raise SystemExit("control header footprint missing")
    if text.count(f'(footprint "{PWR_FP}")') != 2:
        raise SystemExit("expected the power header and the speaker header")
    if text.count('(footprint "Package_DIP:DIP-24_W7.62mm")') != 4:
        raise SystemExit("expected 4 MAX7219 footprints")
    if text.count(f'(footprint "{MATRIX_FP}")') != 4:
        raise SystemExit("expected 4 matrix footprints")
    if text.count('(name "+5V")') != 1:
        raise SystemExit("expected one global +5V net")
    if text.count('(name "GND")') != 1:
        raise SystemExit("expected one global GND net")
    vcc = nodes("+5V")
    gnd = nodes("GND")
    if ("J1", "3") not in vcc or ("J1", "4") not in vcc or ("J1", "16") not in vcc:
        raise SystemExit("+5V does not reach three cart socket contacts")
    if ("J2", "2") not in vcc:
        raise SystemExit("+5V does not reach the header")
    if ("J3", "2") not in vcc:
        raise SystemExit("+5V does not reach the power header")
    drivers_on_vcc = {ref for ref, pin in vcc if pin == "19"}
    if drivers_on_vcc != {f"U{n}" for n in range(2, 6)}:
        raise SystemExit(f"+5V drivers {sorted(drivers_on_vcc)}")
    for pad in ("1", "15"):
        if ("J1", pad) not in gnd:
            raise SystemExit(f"GND does not reach cart socket pad {pad}")
    if ("J1", "2") in gnd:
        raise SystemExit("cart socket pad 2 is EVENT")
    if ("J2", "1") not in gnd or ("J2", "8") not in gnd:
        raise SystemExit("GND does not reach both header ground pins")
    if ("J3", "1") not in gnd or ("J3", "4") not in gnd:
        raise SystemExit("GND does not reach both power header ground pins")
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
    reset_net = nodes("RESET")
    if reset_net != {("J1", "12"), ("J3", "3"), ("U6", "18")}:
        raise SystemExit(f"RESET nodes {reset_net}")
    if text.count('(value "Audio AVR")') != 1 or text.count('(description "AVR128DB28-I/SP")') != 1:
        raise SystemExit("audio AVR missing")
    if text.count('(value "LM386")') != 1 or text.count('(description "LM386N-1 speaker amplifier")') != 1:
        raise SystemExit("LM386 missing")
    if text.count(f'(footprint "{AUDIO_FP}")') != 1:
        raise SystemExit("audio AVR footprint missing")
    if text.count(f'(footprint "{AMP_FP}")') != 1:
        raise SystemExit("LM386 footprint missing")
    if text.count(f'(footprint "{UPDI_FP}")') != 1:
        raise SystemExit("audio UPDI header missing")
    if text.count('(value "10k EVENT")') != 1 or text.count('(value "10k DAC")') != 1:
        raise SystemExit("expected the EVENT pull-up and the DAC load")
    if text.count('(value "22k IN")') != 1 or text.count('(value "1k IN")') != 1:
        raise SystemExit("input divider missing")
    if text.count('(value "10R ZOB")') != 1 or text.count('(value "47n ZOB")') != 1:
        raise SystemExit("zobel missing")
    if text.count('(value "1u DAC")') != 1 or text.count('(value "220u SPK")') != 1:
        raise SystemExit("coupling caps missing")
    if text.count('(value "U6 VDD")') != 1 or text.count('(value "U6 AVDD")') != 1:
        raise SystemExit("U6 bypass caps missing")
    if text.count('(value "U6 IO")') != 1:
        raise SystemExit("U6 IO bypass missing")
    if text.count('(value "U7 VS")') != 2 or text.count('(value "U7 BYP")') != 1:
        raise SystemExit("U7 supply caps missing")
    event = nodes("EVENT")
    if event != {("J1", "2"), ("U6", "23"), ("R1", "1")}:
        raise SystemExit(f"EVENT nodes {event}")
    if ("R1", "2") not in vcc:
        raise SystemExit("EVENT pull-up does not reach +5V")
    dac = nodes("DAC")
    if dac != {("U6", "12"), ("R2", "1"), ("C1", "1")}:
        raise SystemExit(f"DAC nodes {dac}")
    if ("R2", "2") not in gnd:
        raise SystemExit("DAC load does not reach GND")
    aud = nodes("AUD")
    if aud != {("C1", "2"), ("R3", "1")}:
        raise SystemExit(f"AUD nodes {aud}")
    audin = nodes("AUDIN")
    if audin != {("R3", "2"), ("R4", "1"), ("U7", "3")}:
        raise SystemExit(f"AUDIN nodes {audin}")
    if ("R4", "2") not in gnd or ("U7", "2") not in gnd or ("U7", "4") not in gnd:
        raise SystemExit("LM386 grounds missing")
    if ("U7", "6") not in vcc:
        raise SystemExit("LM386 supply missing")
    for num in ("20", "14", "6"):
        if ("U6", num) not in vcc:
            raise SystemExit(f"audio AVR pin {num} is not on +5V")
    for num in ("15", "21"):
        if ("U6", num) not in gnd:
            raise SystemExit(f"audio AVR pin {num} is not ground")
    for ref in ("C4", "C6", "C7", "C8", "C5"):
        if (ref, "1") not in vcc or (ref, "2") not in gnd:
            raise SystemExit(f"{ref} is not across +5V and GND")
    bypass_net = nodes("BYPASS")
    if bypass_net != {("U7", "7"), ("C9", "1")}:
        raise SystemExit(f"BYPASS nodes {bypass_net}")
    if ("C9", "2") not in gnd:
        raise SystemExit("LM386 bypass return missing")
    if "NetTie" in text or '(ref "NT' in text:
        raise SystemExit("net ties must stay out of the netlist")
    amp = nodes("AMP")
    if amp != {("U7", "5"), ("C2", "1"), ("R5", "1")}:
        raise SystemExit(f"AMP nodes {amp}")
    zobel = nodes("ZOBEL")
    if zobel != {("R5", "2"), ("C3", "1")}:
        raise SystemExit(f"ZOBEL nodes {zobel}")
    if ("C3", "2") not in gnd:
        raise SystemExit("zobel cap does not reach GND")
    spk = nodes("SPK")
    if spk != {("C2", "2"), ("J4", "2"), ("J4", "3")}:
        raise SystemExit(f"SPK nodes {spk}")
    if ("J4", "1") not in gnd or ("J4", "4") not in gnd:
        raise SystemExit("speaker return missing")
    audio_updi = nodes("AUDIO_UPDI")
    if audio_updi != {("U6", "19"), ("J5", "1")}:
        raise SystemExit(f"AUDIO_UPDI nodes {audio_updi}")
    if ("J5", "2") not in vcc or ("J5", "3") not in gnd:
        raise SystemExit("audio UPDI header power missing")
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
