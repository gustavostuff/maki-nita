#!/bin/sh
set -eu
cd "$(dirname "$0")"
make -C tools/acetate_sim
exec ./tools/acetate_sim/acetate-sim
