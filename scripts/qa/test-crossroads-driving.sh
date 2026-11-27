#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p build
CC="${CC:-gcc}"
"$CC" -std=c11 -g -O0 -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie \
  scripts/qa/crossroads_driving_test.c -o build/crossroads_driving_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ./build/crossroads_driving_test
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
python3 scripts/qa/parkway_replay.py build/DeadChannel
