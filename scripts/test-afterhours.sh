#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
tmp=$(mktemp -d)
trap 'test -z "$tmp" || find "$tmp" -delete' EXIT
"${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie \
  scripts/qa/night_world_test.c src/linux_save.c -o build/night_world_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/night_world_test "$tmp"
"${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie \
  scripts/qa/afterhours_chain.c src/linux_save.c -o build/afterhours_chain
mkdir -p "$tmp/chain"
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/afterhours_chain "$tmp/chain"
"${CC:-gcc}" -std=c11 -O2 -fPIC -shared scripts/qa/linux_replay.c -ldl -o build/linux_replay.so
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
python3 scripts/qa/afterhours_host.py build/DeadChannel build/linux_replay.so
for test in night_phone_test night_garage_test night_venues_test chase_test night_garage_complete_test; do
  "${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie "src/$test.c" -o "build/$test"
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 "build/$test"
done
"${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie scripts/qa/night_club_test.c src/linux_save.c -o build/night_club_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/night_club_test "$tmp/club"
"${CC:-gcc}" -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie scripts/qa/outer_city_test.c src/linux_save.c -o build/outer_city_test
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 build/outer_city_test "$tmp/outer"
