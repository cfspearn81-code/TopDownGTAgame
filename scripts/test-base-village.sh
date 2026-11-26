#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
tmp=$(mktemp -d)
trap 'test -z "$tmp" || find "$tmp" -delete' EXIT
cc=${CC:-gcc}
flags=(-std=c11 -O0 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie)
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
export SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
DC_QA_BUILT=1 bash scripts/test-linux.sh
python3 scripts/qa/afterhours_host.py build/DeadChannel build/linux_replay.so
"$cc" "${flags[@]}" -DDC_LINUX_NATIVE src/vehicle_durability_test.c src/linux_save.c -o build/vehicle_durability_test
XDG_DATA_HOME="$tmp/durability" build/vehicle_durability_test
"$cc" "${flags[@]}" scripts/qa/base_club_walk.c src/linux_save.c -o build/base_club_walk
build/base_club_walk "$tmp/club"
"$cc" "${flags[@]}" src/street_lamps_test.c -o build/street_lamps_test
build/street_lamps_test
"$cc" "${flags[@]}" -DDC_LINUX_NATIVE src/night_trap_test.c src/linux_save.c -o build/night_trap_test
XDG_DATA_HOME="$tmp/trap" build/night_trap_test
python3 scripts/qa/trap_host.py build/DeadChannel build/linux_replay.so build/night_trap_test
