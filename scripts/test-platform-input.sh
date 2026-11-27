#!/usr/bin/env bash
# Windows runs the actual PE through the Linux Win32-service shim; Linux runs
# its native executable. All career data here is disposable.
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ${DC_QA_BUILT:-0} != 1 ]]; then bash scripts/build-linux.sh; bash scripts/build.sh; fi
tmp=$(mktemp -d); trap 'rm -rf -- "$tmp"' EXIT
cc -O2 scripts/check_pe.c -o "$tmp/pe"
cc -std=c11 -O2 -fPIC -shared scripts/qa/linux_pointer_rate.c -ldl -o "$tmp/mouse.so"
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy XDG_DATA_HOME="$tmp/data" LD_PRELOAD="$tmp/mouse.so" build/DeadChannel
cc -std=c11 -O2 -fPIC -shared scripts/qa/linux_device_reset.c -ldl -o "$tmp/reset.so"
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy XDG_DATA_HOME="$tmp/reset" LD_PRELOAD="$tmp/reset.so" build/DeadChannel
"$tmp/pe" build/DeadChannel.exe - "$tmp/seed"
cc -std=c11 -O2 -fPIC -shared scripts/qa/linux_close_pair.c -ldl -o "$tmp/close.so"
mkdir -p "$tmp/close/DeadChannel"; cp "$tmp/seed" "$tmp/close/DeadChannel/career.dat"
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy XDG_DATA_HOME="$tmp/close" LD_PRELOAD="$tmp/close.so" build/DeadChannel
DC_CLOSE_TEST=1 "$tmp/pe" build/DeadChannel.exe "$tmp/seed"
DC_CLOSE_TEST=2 "$tmp/pe" build/DeadChannel.exe "$tmp/seed"
gzip -dc "${DC_OPENING_TOUR:-scripts/qa/curb-lamp-tour.txt.gz}" | DC_TAP_REPLAY=1 "$tmp/pe" build/DeadChannel.exe --tour
python3 scripts/qa/win_presentation.py
python3 scripts/qa/linux_audio_reconnect.py
