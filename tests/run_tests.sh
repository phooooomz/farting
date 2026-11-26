#!/bin/sh
# Builds and runs host unit tests for the portable core. No PSP SDK needed.
set -e
cd "$(dirname "$0")/.."
OUT="${TMPDIR:-/tmp}/eclipse_tests"
gcc -std=gnu99 -Wall -Wextra -Wno-unused-parameter -I src/core src/core/*.c tests/test_core.c -o "$OUT"
"$OUT"
