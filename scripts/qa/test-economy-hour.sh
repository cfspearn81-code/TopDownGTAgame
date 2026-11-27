#!/usr/bin/env bash
# Short repeatable regression pack from the six-lane playtest. It does not
# purport to be an hour-long soak itself. No existing career files are opened.
set -euo pipefail
cd "$(dirname "$0")/../.."
tmp=$(mktemp -d)
trap 'rm -r -- "$tmp"' EXIT
[[ -x build/DeadChannel && -f build/DeadChannel.exe ]] || { bash scripts/build.sh; bash scripts/build-linux.sh; }
cc -std=c11 -O1 -fno-pie -no-pie -fno-omit-frame-pointer -g -fsanitize=address,undefined,float-cast-overflow \
   -fno-sanitize-recover=all scripts/qa/economy_hour_test.c -o "$tmp/economy"
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 "$tmp/economy" 135
cc -O2 scripts/check_pe.c -o "$tmp/pe-host"
cc -std=c11 -O2 -fPIC -shared scripts/qa/linux_replay.c -ldl -o "$tmp/replay.so"
python3 scripts/qa/economy_hour_hosts.py "$tmp/pe-host" build/DeadChannel.exe build/DeadChannel "$tmp/replay.so"
if [[ ${1:-} == --full-properties ]]; then
   cc -std=c11 -O2 -Isrc scripts/qa/economy_property_route.c -o "$tmp/routes"
   python3 scripts/qa/economy_property_hosts.py "$tmp/routes" "$tmp/pe-host" build/DeadChannel.exe build/DeadChannel "$tmp/replay.so"
fi
