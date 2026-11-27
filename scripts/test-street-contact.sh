#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/contact-qa
out=build/contact-qa
build_required=${DC_QA_BUILT:-0}
export DC_OPENING_TOUR="$PWD/scripts/qa/curb-lamp-tour.txt.gz" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 DC_QA_BUILT=1
pids=()
tag() {
  local n=$1; shift
  (if "$@" >"$out/$n.log" 2>&1; then echo "PASS $n"; else echo "FAIL $n"; tail -40 "$out/$n.log"; exit 1; fi) &
  pids+=("$!")
}
join() { local bad=0; for p in "${pids[@]}"; do wait "$p" || bad=1; done; pids=(); return "$bad"; }
if [[ $build_required != 1 ]]; then
  tag linux-build bash scripts/build-linux.sh
  tag windows-build bash scripts/build.sh
  join
fi
sha256sum build/DeadChannel{,.exe}
tag opening-linux bash scripts/test-linux.sh
tag opening-windows bash -c 'cc -O2 scripts/check_pe.c -o build/contact-qa/pe && gzip -dc "$DC_OPENING_TOUR" | build/contact-qa/pe build/DeadChannel.exe --tour'
tag npc-dual python3 scripts/qa/npc_keyboard_replay.py
tag vault-dual python3 scripts/qa/npc_indoor_replay.py
tag combat-c python3 scripts/qa/chase_encounter_host.py build/DeadChannel
tag downtown-c python3 scripts/qa/chase_city_host.py
tag outlying-c python3 scripts/qa/outer_taxi_replay.py --3d --radio
tag native-vault-c python3 scripts/qa/chase_indoor_host.py build/DeadChannel
tag physical-rooms-native python3 scripts/qa/physical_room_host.py build/DeadChannel
tag physical-outdoor-native python3 scripts/qa/outdoor_cast_host.py build/DeadChannel
tag outer-lamps-route python3 scripts/qa/outer_lamp_host.py
tag lamp-impact-native python3 scripts/qa/street_lamp_hit_host.py
tag street-crossroads bash scripts/test-street-crossroads.sh
testsrc() {
  local p=$1; local n=${p##*/}; n=${n%.c};
  cc -std=c11 -O0 -g -fsanitize=address,undefined,float-cast-overflow \
    -fno-pie -no-pie -I src "$p" -o "$out/$n"
  "$out/$n"
}
for t in src/{street_lamps_test,chase_rooms_test,street_motion_test,npc_hold_test}.c scripts/qa/{outer_lamp_placement_test,street_lamp_solid_test,street_npc_stability,outdoor_cast_test,physical_room_people}.c; do
 n=${t##*/}; n=${n%.c};tag "$n" testsrc "$t"
done
join
