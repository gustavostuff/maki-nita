#!/bin/sh
set -eu
cd "$(dirname "$0")"
make -C tools/sim
exec ./tools/sim/makineat-ui
