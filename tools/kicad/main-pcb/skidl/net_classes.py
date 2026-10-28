"""KiCad net classes for the Maki-Nita console.

AUDIO is the sensitive analog path around U5 and U6. MATRIX is
the display SPI bus and every segment and digit net from the MAX7219
drivers to their 8x8 blocks. Other nets stay in Default. GND is the pour.
"""

from __future__ import annotations

import json
import re
from typing import Iterable

CLASS_AUDIO = "AUDIO"
CLASS_MATRIX = "MATRIX"

AUDIO_NETS = frozenset(
    ("DAC", "AUD", "AUDIN", "AMP", "SPK", "ZOBEL", "BYPASS")
)
SPI_NETS = frozenset(
    ("CLK", "DIN", "LOAD", "DOUT", "DOUT_U1", "DOUT_U2", "DOUT_U3")
)

_GEOM = {
    "bus_width": 12,
    "clearance": 0.2,
    "diff_pair_gap": 0.25,
    "diff_pair_via_gap": 0.25,
    "diff_pair_width": 0.2,
    "line_style": 0,
    "microvia_diameter": 0.3,
    "microvia_drill": 0.1,
    "track_width": 0.4,
    "tuning_profile": "",
    "via_diameter": 0.8,
    "via_drill": 0.4,
    "wire_width": 6,
}

# AUDIO is green. MATRIX is red/orange.
KICAD_CLASSES = [
    {
        **_GEOM,
        "name": CLASS_AUDIO,
        "pcb_color": "rgb(44, 160, 44)",
        "schematic_color": "rgb(44, 160, 44)",
        "priority": 1,
    },
    {
        **_GEOM,
        "name": CLASS_MATRIX,
        "pcb_color": "rgb(214, 39, 40)",
        "schematic_color": "rgb(214, 39, 40)",
        "priority": 2,
    },
]

PATTERNS = [
    *({"netclass": CLASS_AUDIO, "pattern": name} for name in sorted(AUDIO_NETS)),
    *({"netclass": CLASS_MATRIX, "pattern": name} for name in sorted(SPI_NETS)),
    {"netclass": CLASS_MATRIX, "pattern": "DS1_*"},
    {"netclass": CLASS_MATRIX, "pattern": "DS2_*"},
    {"netclass": CLASS_MATRIX, "pattern": "DS3_*"},
    {"netclass": CLASS_MATRIX, "pattern": "DS4_*"},
]

_MATRIX = re.compile(r"^DS[1-4]_")


def class_for_name(name: str) -> str:
    if name in AUDIO_NETS:
        return CLASS_AUDIO
    if name in SPI_NETS or _MATRIX.match(name):
        return CLASS_MATRIX
    return "Default"


def apply_netclasses(nets: Iterable) -> list[str]:
    """Assign SKiDL NetClass objects. Returns the net names that matched."""
    from skidl import NetClass

    by_name = {}
    for spec in KICAD_CLASSES:
        by_name[spec["name"]] = NetClass(
            spec["name"],
            trace_width=spec["track_width"],
            clearance=spec["clearance"],
            via_dia=spec["via_diameter"],
            via_drill=spec["via_drill"],
            priority=10 + spec["priority"],
        )
    matched = []
    for net in nets:
        name = getattr(net, "name", "") or ""
        cls = class_for_name(name)
        if cls == "Default":
            continue
        net.netclasses = by_name[cls]
        matched.append(name)
    return sorted(set(matched))


def annotate_netlist_classes(text: str) -> str:
    """Force one (class ...) on each net in a KiCad sexp netlist.

    SKiDL 2.3 also attaches Default, and a joined class name makes KiCad
    drop the assignment. Only the name that follows a net code is a net name.
    """
    class_re = re.compile(
        r'(\(code\s+\d+\)\s*)\(name "([^"]+)"\)(?:\s*\(class "[^"]*"\))?'
    )

    def repl(match: re.Match) -> str:
        name = match.group(2)
        return f'{match.group(1)}(name "{name}") (class "{class_for_name(name)}")'

    return class_re.sub(repl, text)


def assignments_for(names: Iterable[str]) -> dict[str, str]:
    """Map each net that is not Default onto its class."""
    out = {}
    for name in names:
        cls = class_for_name(name)
        if cls != "Default":
            out[name] = cls
    return dict(sorted(out.items()))


def patch_kicad_pro(text: str, assignments: dict[str, str] | None = None) -> str:
    """Insert or replace the console classes in a .kicad_pro."""
    data = json.loads(text)
    ns = data.get("net_settings") or {}
    classes = [c for c in (ns.get("classes") or []) if c.get("name") == "Default"]
    if not any(c.get("name") == "Default" for c in classes):
        classes.insert(
            0,
            {
                **_GEOM,
                "name": "Default",
                "pcb_color": "rgba(0, 0, 0, 0.000)",
                "schematic_color": "rgba(0, 0, 0, 0.000)",
                "priority": 2147483647,
            },
        )
    classes.extend(dict(spec) for spec in KICAD_CLASSES)
    ns["classes"] = classes
    ns["netclass_patterns"] = list(PATTERNS)
    if assignments is not None:
        existing_assign = ns.get("netclass_assignments") or {}
        uses_list = any(isinstance(v, list) for v in existing_assign.values())
        if uses_list:
            ns["netclass_assignments"] = {k: [v] for k, v in assignments.items()}
        else:
            ns["netclass_assignments"] = assignments
    data["net_settings"] = ns
    start = text.find('"net_settings"')
    if start < 0:
        return json.dumps(data, indent=2) + "\n"
    colon = text.find(":", start)
    obj_start = text.find("{", colon)
    depth = 0
    obj_end = None
    for i in range(obj_start, len(text)):
        ch = text[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                obj_end = i + 1
                break
    if obj_end is None:
        return json.dumps(data, indent=2) + "\n"
    new_obj = _indent_block(json.dumps(data["net_settings"], indent=2), 2)
    return text[:obj_start] + new_obj + text[obj_end:]


def _indent_block(block: str, spaces: int) -> str:
    pad = " " * spaces
    lines = block.splitlines()
    if not lines:
        return block
    out = [lines[0]]
    for line in lines[1:]:
        out.append(pad + line if line else line)
    return "\n".join(out)
