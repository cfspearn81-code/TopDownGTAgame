#!/usr/bin/env python3
"""Real SDL Afterimage cast/guide replay, no private career or display needed.

Run from the source root:
  bash scripts/build-linux.sh
  gcc -std=c11 -O2 -fPIC -shared scripts/qa/linux_replay.c -ldl -o build/linux_replay.so
  python3 scripts/qa/afterimage_host.py build/DeadChannel build/linux_replay.so [existing output directory]
"""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

exe, interposer = [Path(x).resolve() for x in sys.argv[1:3]]
with tempfile.TemporaryDirectory(prefix="dc-afterimage-sdl-") as t:
    tmp = Path(t)
    dest = Path(sys.argv[3]).resolve() if len(sys.argv) > 3 else tmp
    env = dict(os.environ, XDG_DATA_HOME=t, SDL_VIDEODRIVER="dummy",
               SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy",
               LD_PRELOAD=str(interposer), DC_LINUX_REPLAY="club-cast", DC_LINUX_SHOW=str(dest))
    subprocess.run([str(exe)], env=env, check=True, timeout=30)
    load = lambda basename: (dest / (basename + ".ppm")).read_bytes()
    a, b = load("afterimage-stage-a"), load("afterimage-stage-b")
    f, g = load("afterimage-freeze-a"), load("afterimage-freeze-b")
    lounge = load("afterimage-lounge")
    assert a != b, "Stage is not animating"
    assert f == g, "The club changed underneath the field guide"
    assert lounge != b, "VIP entrance did not change the room"
    assert len(load("afterimage-queue")) == len(a), "Missing street queue capture"
    print("Native SDL: fresh exterior queue -> direct E -> live cast -> stable guide -> VIP lounge (six frames).")
