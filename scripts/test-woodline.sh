#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
testdir=$(mktemp -d)
trap 'test -z "$testdir" || find "$testdir" -delete' EXIT
cc=${CC:-gcc}
flags=(-std=c11 -O0 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie)
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
[[ ${DC_QA_BUILT:-0} == 1 ]] || bash scripts/build-linux.sh
DC_QA_BUILT=1 bash scripts/test-base-village.sh
"$cc" "${flags[@]}" src/headlamps_test.c -o build/woodline-headlamps
build/woodline-headlamps
python3 scripts/qa/headlamps_host.py build/DeadChannel build/linux_replay.so build/woodline-headlamps "$testdir/lamps"
"$cc" "${flags[@]}" -DDC_LINUX_NATIVE src/night_forest_test.c src/linux_save.c -o build/woodline-forage
XDG_DATA_HOME="$testdir/forage" build/woodline-forage
python3 scripts/qa/woodline_host.py build/DeadChannel build/linux_replay.so build/woodline-forage
"$cc" "${flags[@]}" scripts/qa/night_club_test.c src/linux_save.c -o build/woodline-club
build/woodline-club "$testdir/club"
python3 scripts/qa/afterimage_host.py build/DeadChannel build/linux_replay.so
"$cc" "${flags[@]}" src/woodline_terrain_test.c -o build/woodline-terrain
build/woodline-terrain
"$cc" "${flags[@]}" scripts/qa/woodline_save_test.c src/linux_save.c -o build/woodline-terrain-save
XDG_DATA_HOME="$testdir/terrain" build/woodline-terrain-save write
XDG_DATA_HOME="$testdir/terrain" build/woodline-terrain-save read
"$cc" "${flags[@]}" src/street_motion_test.c -o build/woodline-street-motion
build/woodline-street-motion
"$cc" "${flags[@]}" src/woodline_wildlife_test.c -o build/woodline-wildlife
build/woodline-wildlife
"$cc" "${flags[@]}" src/harbor_boats_test.c -o build/woodline-harbor-boats
build/woodline-harbor-boats
python3 scripts/qa/harbor_boats_host.py build/DeadChannel build/linux_replay.so build/woodline-harbor-boats "$testdir/helm"
"$cc" "${flags[@]}" scripts/qa/harbor_geometry_test.c -o build/woodline-harbor-geometry
build/woodline-harbor-geometry
"$cc" "${flags[@]}" -DDC_LINUX_NATIVE src/harbor_jobs_test.c src/linux_save.c -o build/woodline-harbor-jobs
XDG_DATA_HOME="$testdir/harbor-jobs" build/woodline-harbor-jobs
XDG_DATA_HOME="$testdir/harbor-base" build/woodline-harbor-jobs --bump-base
python3 scripts/qa/harbor_jobs_host.py build/DeadChannel build/linux_replay.so build/woodline-harbor-jobs
