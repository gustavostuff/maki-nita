#!/bin/sh
set -eu
cd "$(dirname "$0")"
make -C sim
make -C sim test
