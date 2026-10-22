"""KiCad net classes for the Maki Nita field.

Two classes split the matrix wiring by block row, top to bottom, so the
ratsnest can be routed as two bands. The third class is the 5 V rail and
ground. Other nets stay in Default.
"""

from __future__ import annotations

import json
import re
from typing import Iterable

CLASS_ROWS = ("ROW1", "ROW2")
CLASS_POWER = "5V_GND"

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

# ROW1 is the top block row (screen rows 0-7, drivers U2 and U3).
_ROW_COLORS = (
    "rgb(214, 39, 40)",
    "rgb(31, 119, 180)",
)

KICAD_CLASSES = [
    {
        **_GEOM,
        "name": name,
        "pcb_color": color,
        "schematic_color": color,
        "priority": index + 1,
    }
    for index, (name, color) in enumerate(zip(CLASS_ROWS, _ROW_COLORS))
]
KICAD_CLASSES.append(
    {
        **_GEOM,
        "name": CLASS_POWER,
        "pcb_color": "rgb(148, 103, 189)",
        "schematic_color": "rgb(148, 103, 189)",
        "priority": 3,
        "track_width": 0.8,
        "clearance": 0.3,
        "via_diameter": 1.2,
        "via_drill": 0.6,
    }
)

PATTERNS = [
    {"netclass": name, "pattern": f"{name}_*"} for name in CLASS_ROWS
]
PATTERNS.extend(
    [
        {"netclass": CLASS_POWER, "pattern": "+5V"},
        {"netclass": CLASS_POWER, "pattern": "GND"},
    ]
)

_ROW = re.compile(r"^ROW([12])_")


def class_for_name(name: str) -> str:
    if name in ("+5V", "GND"):
        return CLASS_POWER
    match = _ROW.match(name)
    if match:
        return f"ROW{match.group(1)}"
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


def patch_kicad_pro(text: str) -> str:
    """Insert or replace the five Maki Nita classes in a .kicad_pro."""
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
                "track_width": 0.2,
            },
        )
    classes.extend(dict(spec) for spec in KICAD_CLASSES)
    ns["classes"] = classes
    ns["netclass_patterns"] = list(PATTERNS)
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
