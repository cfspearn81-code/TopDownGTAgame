#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash scripts/build.sh
for t in test profiles_test economy_test; do
  gcc -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow \
    -fno-omit-frame-pointer -fno-pie -no-pie "src/$t.c" -o "build/$t"
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 "./build/$t"
done
gcc -O2 scripts/check_pe.c -o build/check_pe
build/check_pe build/DeadChannel.exe - build/test-fixture.dat
DC_OPENING=2 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_DELETE=1 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_POINTER=1 build/check_pe build/DeadChannel.exe build/test-fixture.dat
DC_POINTER=2 build/check_pe build/DeadChannel.exe build/test-fixture.dat
if [[ "${1:-}" == --soak ]]; then
  gcc -std=c11 -O1 -g -fsanitize=address,undefined,float-cast-overflow \
    -fno-omit-frame-pointer -fno-pie -no-pie src/stress_test.c -o build/stress_test
  ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ./build/stress_test
fi
