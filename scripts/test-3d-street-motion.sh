#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
export DC_QA_BUILT=1 ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
mkdir -p build/street-motion-qa
out=build/street-motion-qa
bash scripts/build-linux.sh
bash scripts/build.sh
"${CC:-cc}" -O2 scripts/check_pe.c -o "$out/pe"
gzip -dc scripts/qa/curb-access-tour.txt.gz | "$out/pe" build/DeadChannel.exe --tour
bash scripts/test-linux.sh
python3 scripts/qa/chase_city_host.py
python3 scripts/qa/chase_fleet_host.py build/DeadChannel
python3 scripts/qa/chase_indoor_host.py build/DeadChannel
python3 scripts/qa/chase_gait_native.py build/DeadChannel
python3 scripts/qa/outer_taxi_replay.py --3d --radio
python3 scripts/qa/parkway_replay.py build/DeadChannel
python3 scripts/qa/outer_lamp_host.py
bash scripts/test-street-crossroads.sh
tests=(src/street_lamps_test.c src/street_motion_test.c src/chase_room_rig_test.c src/chase_nature_test.c src/chase_rooms_test.c scripts/qa/outer_lamp_placement_test.c scripts/qa/street_npc_stability.c scripts/qa/chase_drive_motion_test.c scripts/qa/chase_gait_test.c scripts/qa/chase_route_props_test.c)
for src in "${tests[@]}"; do
   name=${src##*/}
   bin="$out/${name%.c}"
   "${CC:-cc}" -std=c11 -O1 -fsanitize=address,undefined,float-cast-overflow \
       -fno-pie -no-pie -I src "$src" -o "$bin"
   "$bin"
done
python3 scripts/qa/chase_route_props_host.py build/DeadChannel build/linux_replay.so \
    "$out/chase_route_props_test" "$out/vendor"
