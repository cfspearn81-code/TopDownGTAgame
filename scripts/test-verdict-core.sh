#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build.sh
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
gcc -O2 scripts/qa/verdict_a1_plan.c -o build/a1plan
gcc -O2 scripts/check_pe.c -o build/check_pe
gcc -shared -fPIC -O2 scripts/qa/linux_replay.c -ldl -o build/linux_replay.so
gcc -std=c11 -O1 -ggdb -fsanitize=address,undefined -no-pie -fno-pie src/verdict_core_test.c -o build/core_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/core_test
python3 scripts/qa/verdict_a1_replay.py
