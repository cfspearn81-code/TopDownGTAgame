#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
"${CC:-gcc}" -std=c11 -O2 -fPIC -shared scripts/qa/linux_replay.c -ldl -o build/linux_replay.so
"${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  scripts/qa/linux_save_test.c src/linux_save.c -o build/linux_save_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  python3 scripts/qa/linux_boot.py build/DeadChannel build/linux_replay.so build/linux_save_test
