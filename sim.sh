#!/bin/sh
set -eu
cd "$(dirname "$0")"
make -C tools/sim
exec ./tools/sim/maki-nita-ui
