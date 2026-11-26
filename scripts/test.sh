#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash scripts/build.sh
for t in test profiles_test economy_test road_test visual_test interior_routes_test bank_ui_test; do
  gcc -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow \
    -fno-omit-frame-pointer -fno-pie -no-pie "src/$t.c" -o "build/$t"
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 "./build/$t"
done
gcc -std=c11 -O1 -ffunction-sections -fdata-sections -fshort-wchar -Wl,--gc-sections \
    scripts/qa/save_disk.c -o build/save_disk
build/save_disk
gcc -O2 scripts/check_pe.c -o build/check_pe
build/check_pe build/DeadChannel.exe - build/test-fixture.dat
python3 scripts/qa/legacy_saves.py build/check_pe build/DeadChannel.exe build/test-fixture.dat
python3 scripts/qa/atm_boot.py build/check_pe build/DeadChannel.exe build/test-fixture.dat
python3 scripts/qa/deadline_boot.py build/check_pe build/DeadChannel.exe build/test-fixture.dat
python3 scripts/qa/midnight_boot.py build/check_pe build/DeadChannel.exe build/test-fixture.dat
python3 scripts/qa/lantern_crew_boot.py build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_OPENING=2 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_DELETE=1 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_POINTER=1 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_POINTER=2 build/check_pe build/DeadChannel.exe build/test-fixture.dat
if [[ "${1:-}" == --soak || "${1:-}" == --extended ]]; then
  gcc -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow \
    -fno-omit-frame-pointer -fno-pie -no-pie src/stress_test.c -o build/stress_test
  soak_args=()
  if [[ "$1" == --extended ]]; then soak_args+=(--extended); fi
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ./build/stress_test "${soak_args[@]}"
fi
if [[ "${1:-}" == --tour ]]; then
  gzip -dc scripts/qa/easy-entry-tour.txt.gz | build/check_pe build/DeadChannel.exe --tour
fi
if [[ "${1:-}" == --historical-tour ]]; then
  # Play the archived recording against the executable it was designed for.
  old_exe=${2:?Supply the absolute path to the Inside Port Vanta DeadChannel.exe}
  gzip -dc scripts/qa/street-tour.txt.gz | build/check_pe "$old_exe" --tour
fi
