#!/usr/bin/env bash
# Write the console netlist from the SKiDL circuit.
set -euo pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"
NET="$DIR/maki-nita.net"

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    echo "Usage: $0"
    echo "  Writes $NET"
    echo "  The cartridge netlist is kicad/cart-pcb/skidl/export_netlist.sh"
    exit 0
fi
if [[ $# -ne 0 ]]; then
    echo "unknown option: $1" >&2
    exit 2
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 not found" >&2
    exit 1
fi
if ! python3 -c "import importlib.util, sys; sys.exit(0 if importlib.util.find_spec('skidl') else 1)"; then
    echo "skidl not installed" >&2
    exit 1
fi

python3 "$DIR/maki_nita.py"
