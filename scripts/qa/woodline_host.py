"""Native SDL smoke checks against separate temporary careers, never player data."""
from pathlib import Path
import os, struct, subprocess, sys, tempfile

exe, preload, fixture = map(lambda p: Path(p).resolve(), sys.argv[1:])
UP, DOWN, RIGHT, E, ESC, F5, ENTER, MAP, P = 1, 2, 8, 16, 128, 512, 1024, 256, 524288
GARAGE = 304 + 32 * 4
def plan(*events):
    out = [(1, 0)]
    for v in events:
        if type(v) is tuple:
            out.append(v)
        else:
            out.extend(((1, v), (1, 0)))
    return (f"{sum(n for n,_ in out)} 0\n" + "".join(f"{n} {v}\n" for n,v in out)).encode()
def run_case(how, actions):
    with tempfile.TemporaryDirectory(prefix="dc-woodline-host-") as temp:
        env = dict(os.environ, XDG_DATA_HOME=temp, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software", SDL_AUDIODRIVER="dummy")
        clean_env = {k:v for k,v in env.items() if k != "LD_PRELOAD"}
        subprocess.run([fixture, "--seed", how], env=clean_env, timeout=35, check=True)
        env.update(LD_PRELOAD=str(preload), DC_LINUX_REPLAY="")
        save = Path(temp) / "DeadChannel" / "career.dat"
        def run(*events):
            subprocess.run([exe], env=env, input=plan(*events), check=True, timeout=90)
            b = save.read_bytes()
            assert len(b) == 1044 and struct.unpack_from("<II", b) == (0x44433131,11)
            h = 2166136261
            for ch in b[:-4]: h = ((h ^ ch) * 16777619) & 0xffffffff
            assert h == struct.unpack_from("<I",b,len(b)-4)[0]
            return struct.unpack_from("<i",b,8)[0], struct.unpack_from("<64I",b,GARAGE)
        actions(run)
def ranger(run):
    cash, slots = run(ENTER, P, RIGHT, RIGHT, UP, ENTER, P, (34, DOWN|RIGHT), (47, DOWN), E, E, DOWN, E, MAP, ESC, F5)
    assert cash == 2000 and slots[49] == 5 and slots[50] == 0
def patch(run):
    cash, slots = run(ENTER, E, E, F5)
    assert cash == 2000 and slots[49] == 1<<12 and slots[50] == 1
    assert run(ENTER, E, F5)[1][50] == 1
def planter(shop):
    def route(run):
        walk = (ENTER, E, (38,RIGHT), (19,UP), ENTER)
        cash, slots = run(*walk, E, E, ESC, F5)
        assert cash == 1995 and slots[51+shop]&7 == 1 and (slots[51+shop]>>3)>850
        cash, slots = run(*walk, ESC, (910,0), ENTER, E, F5)
        assert (cash,slots[50],slots[51+shop]) == (1995,1,0)
    return route
run_case("-2",ranger)
run_case("0",patch)
run_case("p0",planter(0))
run_case("p1",planter(1))
print("PASS Linux SDL Woodline: Signal pin, Seawall entry on foot, ranger start, collectible reload no duplicate, physical benches in both shops, saved crop grows and once only harvests.")
