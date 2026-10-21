#!/bin/sh
set -eu
cd "$(dirname "$0")"
make -C sim
make -C sim test
if command -v avr-gcc >/dev/null 2>&1; then
    make -C fw/avr
else
    echo "avr-gcc not found, skipping fw/avr/square.hex"
fi
