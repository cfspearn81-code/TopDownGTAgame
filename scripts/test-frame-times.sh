#!/usr/bin/env bash
# Native SDL and actual Windows executable under Win32 stubs, with disposable data.
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ${DC_QA_BUILT:-0} != 1 ]]; then bash scripts/build-linux.sh; bash scripts/build.sh; fi
tmp=$(mktemp -d); trap 'rm -rf -- "$tmp"' EXIT
cc -O2 -std=c11 scripts/qa/frame_probe_test.c -o "$tmp/unit"
cc -O2 -std=c11 -fPIC -shared scripts/qa/linux_frame_probe.c -ldl -o "$tmp/linux-probe.so"
cc -O2 scripts/check_pe.c -o "$tmp/check_pe"
"$tmp/unit"
DC_FRAME_EXPORT="$tmp/windows.csv" "$tmp/check_pe" build/DeadChannel.exe
for scenario in auto focus; do
 args=()
 if [[ $scenario == focus ]]; then args+=(DC_PROBE_FOCUS=1); fi
 env "${args[@]}" XDG_DATA_HOME="$tmp/$scenario" SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy LD_PRELOAD="$tmp/linux-probe.so" build/DeadChannel
done
python3 - "$tmp" <<'PYTEST'
from pathlib import Path
import csv, re, sys
root=Path(sys.argv[1])
cols=["frame","interval_us","paint_us","present_call_us"]
for path in (root/"windows.csv",root/"auto/DeadChannel/frame-times.csv",root/"focus/DeadChannel/frame-times.csv"):
    data=path.read_bytes()
    assert len(data)<12064 and re.fullmatch(rb"frame,interval_us,paint_us,present_call_us\r\n(?:\d+,\d+,\d+,\d+\r\n)+",data)
    header,*rows=csv.reader(data.decode("ascii").splitlines())
    assert header==cols and 1<len(rows)<=300
    for i, row in enumerate(rows,1):
        a,b,c,d=map(int,row)
        assert a==i and b>0 and c>0 and d>0
    if path.name!="windows.csv":
        assert path.stat().st_mode & 0o777==0o600
    print(f"{path.relative_to(root)}: {len(rows)} numeric relative timing rows, no profile or input data.")
PYTEST
