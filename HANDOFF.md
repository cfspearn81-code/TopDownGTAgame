# Dead Channel | Project handoff

**Shipped baseline:** Career Files: Save Management and Upright Preview,
2026-11-26. Native Windows 11 x64, offline/single-player. Save format revision 7.
The ZIP in `downloads/` is the complete playable build, not a patch.
The game is an original Port Vanta sandbox, independent of John's Room;
do not substitute a Web build or use Rockstar code/assets.

**Maintenance contract:** Update this document before uploading every new
build, even a hotfix. The next build entry should say what changed, what
was verified, save compatibility, any untested surface, and the matching
download/checksum. The repository rules in [AGENTS.md](AGENTS.md) apply to
human and AI contributors alike.

## Current player experience

- A new file begins in the editor, then Apartment 4, with no gun, car or
  money. Follow the first-shift GPS to Mercury Dispatch, interact with Mara
  and visit the courier counters. The introductory courier trip is untimed.
- Eight distinct jobs: Courier Shift, Taxi Fare, Harbor Recovery, Glass Mile
  Sprint, Vault Run, Hot Property, Signal Raid and Rival Collection. Legal
  jobs pay clean cash; street contracts pay street cash. Touch outdoor job
  checkpoints without pressing E; indoor counters deliberately require E.
  Some gates require the correct vehicle. Don’t make active jobs save by
  accident without designing recovery of their transient state.
- Mercury taxi fares use actual passenger boarding/riding/exiting states.
  Complete the first fare before using T for follow-up dispatch; the game
  varies curb pickup, drop-off and rider without immediate repeats. Fare
  base is locked to road-route distance, and tips depend on safe driving.
- All 78 actual city buildings are enterable: 15 named venues and 63 ordinary
  buildings, the latter using 19 archetypes and address-dependent details.
  Four rear rooms add staff/operations spaces. Eight retired building-array
  entries are not physical buildings. Keep doors reachable from the pavement;
  roofs/camera clipping and interior egress are gameplay, not just art.
- Pedestrians use sidewalk/crosswalk routes. Traffic and parked cars have
  collision. Cars can be stolen, damaged, purchased and repaired at
  Nightglass; headlights are clipped by buildings. The police roster is finite
  (eight officers with HQ and neighborhood posts), not an infinite spawner.
- UI keeps clean cash green, street cash red, including sale notices. Do
  not collapse them into one value. Purchases that draw heavily on street
  cash can trigger Revenue, seizure and a case. Clear the case through
  restitution or a fine; no “wait in prison” loop.
- Legal franchise buy-ins: Mercury Dispatch $360, Ferry Market $710,
  Wrecker Yard $1,200. Passive clean income per 120 active seconds:
  $24/$36/$49. Purchase via Business Office and enter the staff door.
  The fictional desk converts capped game street credits with a 25% fee,
  available to claim at the same desk after simulated closing; no duplicate
  claim. The in-game day runs 480 active seconds. Deposits in the last
  45 seconds defer another day. Audit can freeze or seize credits.
- Lantern Grow Loft $3,000; Cinder Works $10,000. Abstract supply credits,
  three business tiers and 120-second production loops. Grow progress remains
  live under normal gameplay menus/maps; Lantern has a fern-store front and
  growing visuals in the rear. Keep this fictional and abstract, not a
  real-world recipe or money-processing tutorial.

## Input and Career Files

`WASD` / arrows move and steer; `Shift` sprints/handbrakes; `E` interacts
and exits vehicles; `Enter` enters nearby vehicles or confirms in a menu;
`X` punches; `Space` uses fists or the equipped weapon. `T` accepts
another unlocked taxi fare. `M` opens the atlas (left-click pin,
right-click clear). `F1` help, `F2` low FX, `F3` Career Files, `F4`
separate new career, `F5` manual save, `Esc` pause/back.

Each card has the persisted portrait, name, playtime and both cash
balances; selected card shows a larger preview. Character creation has a name,
ten appearance categories, 360 distinct editable presets, shuffle and a
four-way preview. The new **upright preview** uses a different projection
from gameplay and shows the torso without shortening the sprite. Do not
change world collision/camera just to revise the preview. On-foot facing
arrow is offset by projection so tall hair does not overlap it facing north.

**Delete Save** targets the selected card; the dialog names the profile
and on-disk slot number, defaults to cancel, and supports mouse or
Right then Enter. Delete of the currently running career is blocked from the
in-game menu; it can be deleted from the launch screen with the same
confirmation. An unsuccessful delete must not remove other slots or change
the active write target. Users must decide which saves to delete.

## Source map and invariants

| Area | Files / patterns |
|---|---|
| Shared engine | `src/game.c`, `src/game.h`: draw primitives, fixed 60 Hz tick, world, jobs, wallet, save schema, dispatch hooks and scene routing |
| Windows host | `src/windows.c`: Win32 events, keyboard and WM_CHAR, mouse / letterbox coordinates, fixed step, native DIB and disk writes |
| City & indoor art | `district_art.inc`, `interiors.inc`, `streetside_interiors.inc`: venue locations, walk-in layouts, staff rooms, collision and door transitions |
| Roads & law | `street_life.inc`, `dispatch.inc`, `navigation.inc`: traffic, sidewalks, pursuit and road/GPS graph; overview plus interactive cursor |
| Career UI | `profiles.inc`, `character.inc`: catalogue, editor, deletion guard, portraits and layered style choices; renderer is shared with the world |
| Business | `legal_business.inc`, production and wallet paths in `game.c`: elapsed timers, save triggers, audit and notices |
| Build / tests | `scripts/build.sh`, `scripts/coff.py`, `scripts/check_pe.c`, `scripts/test.sh`, and `src/*test.c` |

The game uses a 960x540 software framebuffer and 2048x2048 world grid.
The `.inc` files are implementation fragments included by `game.c`; don’t
compile them separately. The Windows shell builds freestanding with a Win64
ABI on x86-64 Linux, converts object files with `coff.py`, and links with
GNU ld's i386pep emulation. Keep Win32 import declarations in sync with the
import list in `coff.py` and test bindings in `check_pe.c`. No additional
game engine, Microsoft runtime bundle, network, or asset directory is needed.
The generated `font.inc` is committed; regenerate only if needed and retain
`docs/FONT-LICENSE.txt`.

Saves are in `%LOCALAPPDATA%/DeadChannel`: `career.dat`, historical
`opening.dat`, then `fresh-001.dat` through `fresh-250.dat`. Exact-size reads,
magic/revision checks, ranges and checksum guard the format. Rev7 imports
revisions 2–6; old playtime was not captured and shows “Untracked” or
“Since update”. Changing struct layout requires an explicit new revision
and compatibility tests. Replacement writes use a temporary file and an
atomic rename; new careers use no-replace rename and must not clobber existing
or unreadable disks. Leave the existing disk path selected after a failed
operation. Do not use actual player saves as test fixtures.

## Verification and remaining work

`bash scripts/test.sh` checks new career, portraits/presets, save migrations,
deletion and failure behavior, gameplay through all doors, business and wallet
state, and the exact generated PE via simulated Win32. Use `--soak` for the
180,000-tick sanitized stress run. See the zipped prior QA reports for the
City Interiors and original Career Files test record. Most recent native-PE
replay exercised delete default cancel, explicit confirm, surviving file load,
fresh creator and letterboxed map. The game test verified 360 distinct previews
and arrow clearance across 128 angles. No physical Windows 11 run was done.

Known gaps, not promises: active jobs and pursuits reset on relaunch or slot
change; progress such as wallet, case, property, stock, vehicle condition and
customization is persistent. Pre-update career time is unavailable. There is
no audio, multiplayer, installer or cloud save sync. Windows verification
should include a real resizable-window/HiDPI session, first courier and taxi,
an indoor menu, clean/street notices and an intentional delete of a disposable
slot. Preserve the working desktop build while experimenting.

## Packaging checklist

1. Update this handoff and player documentation first. Log the release
   name/date, save compatibility, test status, and outstanding problems.
2. Rebuild using the published repository and run the tests. Build ZIP
   layout: `Dead-Channel/DeadChannel.exe`, player/QA notes, optional `Previews/`
   and rebuildable `Source/`. Exclude private metadata and real saves.
3. Run `unzip -tq` and compare the embedded EXE to the tested binary.
   Record `sha256sum` for the ZIP and update README's link.
4. Publish the matching source, handoff and artifact together; check that
   the GitHub path and checksum refer to the actual published file.

Current Windows archive and digest: [downloads/SHA256SUMS.txt](downloads/SHA256SUMS.txt).
