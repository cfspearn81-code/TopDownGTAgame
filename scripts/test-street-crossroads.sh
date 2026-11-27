#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
for t in outer_people_test street_polish_test; do
  "${CC:-cc}" -I src -O1 -g -std=c11 -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie src/"$t".c -o build/"$t"
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/"$t"
done
