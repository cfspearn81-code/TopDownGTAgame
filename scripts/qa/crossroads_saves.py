"""Replay real predecessor and Crossroads ELF saves; optional exact PE replay.

Arguments: upstream_source upstream_elf base_source base_elf merged_source
Uses throwaway saves only. The Base Village starting fixture provides game cash
and stock; its actual old launcher purchases the property and upgrades.
"""
from pathlib import Path
import os
import struct
import subprocess
import sys
import tempfile

old, old_exe, base, base_exe, merged = (Path(x).resolve() for x in sys.argv[1:])
exe = merged / "build/DeadChannel"
ENTER, ESC, LEFT, DOWN, P, F5 = 1024, 128, 4, 2, 524288, 512
FNV = 2166136261


def command(args, **opts):
    return subprocess.run(args, check=True, timeout=95, **opts)


def tape(*masks):
    pairs = [(1, 0)]
    for mask in masks:
        pairs.extend(((1, mask), (2, 0)))
    return (f"{sum(n for n, _ in pairs)} 0\n" +
            "".join(f"{n} {k}\n" for n, k in pairs)).encode()


def fnv(raw):
    h = FNV
    for b in raw:
        h = ((h ^ b) * 16777619) & 0xffffffff
    return h


def get32(raw, off):
    return struct.unpack_from("<I", raw, off)[0]


def check(raw, size, patches):
    assert len(raw) == size and get32(raw, 0) == 0x44433131
    assert get32(raw, 4) == 11 and raw[267] == patches
    assert get32(raw, 300) == fnv(raw[:300])
    if size == 1044:
        assert get32(raw, 1040) == fnv(raw[:1040])
    return raw


with tempfile.TemporaryDirectory(prefix="dc-crossroads-fixture-") as dest:
    work = Path(dest)
    shared = {}
    for name, root in {"old": old, "base": base, "new": merged}.items():
        shared[name] = work / (name + ".so")
        command(["gcc", "-std=c11", "-O2", "-fPIC", "-shared", root / "scripts/qa/linux_replay.c",
                 "-ldl", "-o", shared[name]])

    def env(dir, replay):
        return dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                    SDL_AUDIODRIVER="dummy", XDG_DATA_HOME=str(dir),
                    LD_PRELOAD=str(shared[replay]), DC_LINUX_REPLAY="", DC_TRIP_SEED="17")

    def play(binary, dir, host, plan):
        command([binary], env=env(dir, host), input=plan)

    # Write the compact Rev11 using the predecessor's own market and wheel.
    prev = work / "previous-med"
    slot = prev / "DeadChannel/career.dat"
    slot.parent.mkdir(parents=True)
    planner = work / "wheel-plan"
    command(["gcc", "-std=c11", "-O2", "-I", old / "src",
             old / "scripts/qa/wheel_buy_plan.c", "-o", planner])
    plan = command([planner, slot], stdout=subprocess.PIPE).stdout
    play(old_exe, prev, "old", plan)
    compact = check(slot.read_bytes(), 304, 2)
    assert get32(compact, 8) == 100 and get32(compact, 288) == 87

    play(exe, prev, "new", tape(ENTER, F5))
    kept = check(slot.read_bytes(), 304, 2)
    assert get32(kept, 8) == 100 and get32(kept, 288) == 87

    # Pin a persistent Signal key preference in the same career, producing a
    # checked 1044-byte hybrid with the med count plus a one-time exact backup.
    play(exe, prev, "new", tape(ENTER, P, LEFT, ENTER, ESC, F5))
    hybrid = check(slot.read_bytes(), 1044, 2)
    assert get32(hybrid, 688) == 1 and get32(hybrid, 684) & 0x80000000
    snaps = sorted(slot.parent.glob(".pre-crossroads-career.dat-r11-*.bak"))
    assert len(snaps) == 1 and snaps[0].read_bytes() == kept
    play(exe, prev, "new", tape(ENTER, F5))
    assert get32(check(slot.read_bytes(), 1044, 2), 688) == 1
    assert sorted(slot.parent.glob(".pre-crossroads-career.dat-r11-*.bak")) == snaps

    # Write Trap House and old graded stock using the real Base Village launcher.
    village = work / "previous-house"
    seed = work / "trap-seed"
    command(["gcc", "-std=c11", "-O2", "-DDC_LINUX_NATIVE", base / "src/night_trap_test.c",
             base / "src/linux_save.c", "-o", seed])
    command([seed, "--seed"], env=dict(os.environ, XDG_DATA_HOME=str(village)))
    play(base_exe, village, "base",
         tape(ENTER, ENTER, ENTER, ENTER, ENTER, ENTER, DOWN, ENTER, ENTER, DOWN, ENTER, ENTER, F5))
    slot2 = village / "DeadChannel/career.dat"
    house = check(slot2.read_bytes(), 1044, 0)
    assert get32(house, 8) == 1680 and get32(house, 688 + 58*4) == 43
    assert get32(house, 688 + 33*4) == 6
    play(exe, village, "new", tape(ENTER, F5))
    house_kept = check(slot2.read_bytes(), 1044, 0)
    assert get32(house_kept, 8) == 1680
    assert house_kept[688 + 58*4:688+64*4] == house[688 + 58*4:688+64*4]
    assert get32(house_kept, 688 + 33*4) == 6

    pe = merged / "build/DeadChannel.exe"
    if pe.exists():
        pe_harness = work / "check-pe"
        command(["gcc", "-O2", merged / "scripts/check_pe.c", "-o", pe_harness])
        for kind, raw, size, patches, off in (("compact", kept, 304, 2, 288),
                                             ("hybrid", hybrid, 1044, 2, 688),
                                             ("house", house, 1044, 0, 688 + 58*4)):
            initial, ending = work / (kind + "-in.dat"), work / (kind + "-out.dat")
            initial.write_bytes(raw)
            command([pe_harness, pe, "--tour"], input=tape(ENTER, F5),
                    env=dict(os.environ, DC_TRIP_LOAD=str(initial), DC_TRIP_SAVE=str(ending)))
            assert get32(check(ending.read_bytes(), size, patches), off) == get32(raw, off)
        print("Exact PE (simulated Win32) retained compact, combined and Base Village Rev11 records.")
    print("Crossroads saves: predecessor market med patches, real Trap House upgrades, mixed "
          "expanded data and exact one-time compatibility backup all survive native relaunch.")
