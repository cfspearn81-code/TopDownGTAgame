"""Live input to both clients using private synthetic careers; no external dependencies.
Usage: ./script [--before] host game.exe game.elf linux_replay.so
--before asserts actual released defects, default asserts repaired outcomes."""
import os,sys,struct,tempfile,subprocess,random
from pathlib import Path
before="--before" in sys.argv[1:]
seed_value=next((int(x.split("=")[1]) for x in sys.argv[1:] if x.startswith("--seed=")),1)
host,pe,elf,preload=[Path(x).resolve() for x in sys.argv[1:] if not x.startswith("--")]
def tape(keys):
    a=[0]
    for k in keys:a.extend((k,0,0))
    return (str(len(a))+" 0\n"+"".join("1 "+str(k)+"\n" for k in a)).encode()
def fnv(b):
    h=2166136261
    for x in b:h=((h^x)*16777619)&0xffffffff
    return h
def word(b,offset):return struct.unpack_from("<I",b,offset)[0]
def seconds(b,offset):return struct.unpack_from("<f",b,offset)[0]
def run(which,case,seed,keys,tmp):
    t=tmp/which/case;t.mkdir(parents=True);src=t/"seed";out=t/"snapshot";src.write_bytes(seed)
    env=dict(os.environ,DC_TRIP_SEED="17")
    if which=="PE":
        cmd=[host,pe,"--tour"];env.update(DC_TRIP_LOAD=str(src),DC_TRIP_SAVE=str(out))
    else:
        active=t/"xdg/DeadChannel/career.dat";active.parent.mkdir(parents=True);active.write_bytes(seed)
        cmd=[elf];env.update(SDL_VIDEODRIVER="dummy",SDL_RENDER_DRIVER="software",SDL_AUDIODRIVER="dummy",
                            XDG_DATA_HOME=str(t/"xdg"),LD_PRELOAD=str(preload),DC_LINUX_REPLAY="",DC_LINUX_SNAPSHOT=str(out))
    p=subprocess.run(cmd,input=tape(keys),env=env,capture_output=True,timeout=85);assert p.returncode==0,(p.stdout,p.stderr)
    b=out.read_bytes();assert len(b) in (304,1044) and word(b,len(b)-4)==fnv(b[:-4]) and seed==src.read_bytes()
    return b
with tempfile.TemporaryDirectory(prefix="dc-career-wallet-") as t:
    t=Path(t);fixture=t/"compact";s=subprocess.run([host,pe,"-",fixture],capture_output=True,timeout=85);assert s.returncode==0
    cases=("low_hp","small_atm","eight_hour_rest","open_case_rest","full_distributor","partial_settlement")
    for scene in cases+(() if before else ("randomized_atm",)):
        seed=bytearray(fixture.read_bytes())
        def put(i,v):struct.pack_into("<I",seed,i,v)
        def ft(i,v):struct.pack_into("<f",seed,i,v)
        put(8,9);put(12,7);put(172,4);put(288,0);ft(60,388.0);ft(64,736.0)
        keys=[1024,512] if scene=="low_hp" else [1024,16,4,1024,128,512]
        if "rest" in scene:
            keys=[1024,1024,8,8,2,16,512]
            put(172,0);put(8,350);put(136,60);put(288,700);ft(248,1200.0)
            for offset in (200,216,232):put(offset,1)
            # Deposit due at civil midnight; purchased Lantern batch already growing.
            ft(208,240);put(212,75);struct.pack_into("<6if",seed,68,1,0,1,0,120000,0,95.)
            if scene=="open_case_rest":put(148,1);put(296,15000)
        if scene=="full_distributor":
            put(136,2147483645);put(44,0);put(24,0)
            struct.pack_into("<6if",seed,96,1,2,1,2,120000,0,.02);keys=[1024,512]
        elif scene=="partial_settlement":
            put(8,2147483644);put(140,79);ft(164,.02);keys=[1024,512]
        elif scene=="randomized_atm":
            # Model the *displayed* dollar steps, not the implementation's array index.
            prng=random.Random(seed_value);cash=prng.randrange(4,26000);bank=prng.randrange(1,17000);dirty=prng.randrange(1,600)
            put(8,cash);put(288,bank);put(136,dirty);prefs=(1,10,25,50,100,250,500,1000,2500,5000,10000,25000)
            keys=[1024,16];mode=0;choice=100
            for _ in range(60):
                key=prng.choice((1,2,4,4,8,8,16,16,1024,512));keys.append(key)
                available=bank if mode else cash;shown=available if choice is None else min(choice,available)
                if key in (1,2):mode=1-mode
                elif key==4:choice=max((x for x in prefs if x<shown),default=1)
                elif key==8:choice=next((x for x in prefs if shown<x<available),None)
                elif key in (16,1024):
                    if mode:cash+=shown;bank-=shown
                    else:cash-=shown;bank+=shown
            keys.extend((128,512));expected=(cash,bank,dirty)
        put(300,fnv(seed[:300])); outputs=[]
        for which in ("PE","ELF"):
            b=run(which,scene,seed,keys,t);outputs.append(b)
            print(which,scene,"HP",word(b,12),"clean",word(b,8),"dirty",word(b,136),"bank",word(b,288),
                  "clock",round(seconds(b,248),1),"countdown",word(b,296),"Lantern stock",word(b,124),"due",round(seconds(b,208),2),flush=True)
            if scene=="low_hp":assert word(b,12)==(15 if before else 7)
            elif scene=="small_atm":assert (word(b,8),word(b,288))==((0,9) if before else (8,1))
            elif scene=="randomized_atm":assert (word(b,8),word(b,288),word(b,136))==expected
            elif scene=="full_distributor":
                if before:assert word(b,136)==2147483647 and word(b,44)==6 and seconds(b,120)==0
                else:assert word(b,136)==2147483645 and word(b,44)==0 and 0<seconds(b,120)<=.0002
            elif scene=="partial_settlement":
                assert word(b,8)==2147483647 and word(b,140)==(0 if before else 76)
            else:
                assert 240<=seconds(b,248)<241 and word(b,124)==5
                assert seconds(b,208)==0 and word(b,212)==75 and word(b,288)==700
                if scene=="eight_hour_rest":assert word(b,8)==(786 if before else 350)
                elif before:assert word(b,8)<350 and word(b,136)==0
                else:assert (word(b,8),word(b,136))==(350,60) and 14950<word(b,296)<15000
        assert outputs[0]==outputs[1],scene
