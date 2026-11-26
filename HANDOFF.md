# Dead Channel | Project handoff

**Current build** — [Sidewalk Patrols for Windows x64](downloads/Dead-Channel-Sidewalk-Patrols-Windows-11-x64.zip), November 26, 2026.
Officers use the civilian sidewalk
and crossing rules on patrol, on calls, and when returning to their posts.
The red balance is labeled dirty money. Save revision 7 and the revision 2–6
loaders carry forward. The download checksum is in [SHA256SUMS.txt](downloads/SHA256SUMS.txt).

Patrol beats and station posts start on the pavement. Calls and searches
target the nearest sidewalk, and officers finish each crossing before
redirecting. Officers and civilians share crossing checks, vehicle clearance,
and withdrawal to the curb if a car blocks a crossing in progress.

The standard regressions passed, including the new patrol, pursuit, return,
moving-car and blocked-crosswalk cases. The keyboard tour ran 57,765 frames
through the Windows executable with simulated Win32. The final career had
four legal and four street completions, $255 clean cash and $970 dirty money.
The new digest is `664909bd`. The 180,000-tick sanitizer soak passed.
Physical Windows 11 verification is outstanding.

**Previous build** — Street Handling, November 26, 2026. The
[Windows x64 ZIP](downloads/Dead-Channel-Street-Handling-Windows-11-x64.zip)
contains the native game, player notes and rebuildable source. Save revision 7
continues to load careers from revisions 2–6.

## Street Handling

- The camera eases ahead with forward and reverse velocity. Civilian and
  officer respawn checks use that same camera rectangle. The dash shows the
  vehicle model, direction and speed relative to its rated maximum.
- The radar and atlas mark officers assigned to the current pursuit. The HUD
  shows armor and reports the final part of a pursuit as CLEARING. Handoffs
  open when heat reaches zero.
- Re-entering a previously stolen car during a session preserves its theft
  report. It remains ineligible for licensed fares. A nearby boardable car
  takes priority over a faster one. Entry and exit prompts reflect speed.
- A lethal wreck finishes the downed transition before processing interaction.
  The pause menu keeps the atlas closed until gameplay resumes.
- The previous [Port Vanta Contracts ZIP](downloads/Dead-Channel-Port-Vanta-Contracts-Windows-11-x64.zip)
  is available alongside its recorded checksum.

**Maintenance contract:** Update this document before uploading every new
build, even a hotfix. The next build entry should say what changed, what
was verified, save compatibility, any untested surface, and the matching
download/checksum. The repository rules in [AGENTS.md](AGENTS.md) apply to
human and AI contributors alike.

## Port Vanta Contracts build

Job objectives now use the relevant city spaces and actions.

| Contact | Work and completion |
|---|---|
| Mara at Mercury | Sign for two parcels, deliver them at Ferry Market and Nightglass, then return the signed manifest. Taxi passengers board an owned or licensed car at the curb and pay after getting out at the destination. |
| Dax at Wrecker Yard | Restart the marked stalled work van on foot, then park that van in the service bay and hand it over with E. |
| Paz at Glass Mile | The clock starts at the start line. The same licensed car must pass each gate and cross the finish. |
| Ivo at Vault 39 | Collect a lockbox at a parcel counter and lose pursuit before bringing it to Ivo. Hot Property calls for one marked Vectra, which Vera takes at the Kiosk 09 loading bay when the player is clear. |
| Signal Raid | Take the cartridge from the marked radio console and install it in the backup studio. |
| Rival Collection | Clear the guards at the East Freight cache, pick up the consignment with E and deliver it to Vera. |

Lantern batches start at the owner's grow-room terminal. Cinder batches use
the lab station. Ferry Market supplies both facilities and Kiosk 09 buys
their stock. Each warehouse has its own capacity and security. Bulk sales
can cost three units of remaining stock at the source facility. The
distributor handles finished batches for its posted fee.

Breakwater now provides a timed rest and a place to recover when downed.
Tideway Clinic recovers players who haven't acquired Breakwater. Nightglass
repairs a vehicle brought to the shop or the selected garage vehicle.
Treatments and equipment leave the wallet alone when health or armor is
already full. Revenue accepts a clean-cash penalty or three courier and
licensed recovery shifts as community service. Revenue freezes accounts
while its audit timer runs during active play. The three franchise accounts
still pay their own income and settle at their own staff desk after closing.

The save layout is revision 7 and reads revisions 2–6. Existing Breakwater
owners can rest there, legacy lockboxes secure both warehouses, and existing
restitution progress carries forward. Active contracts and rest timers are
session state. See [QA-REPORT.txt](docs/QA-REPORT.txt) for verification and
[SHA256SUMS.txt](downloads/SHA256SUMS.txt) for the Windows archive.

## Previous Port Vanta Playtest build

- Vehicle jobs put a gold marker on their staged car and route GPS to it.
  Hot Property requires the stolen car used at the pickup to reach the
  handoff. After the pickup, GPS points back to that car if the player swaps
  rides. Wrecking the delivery car ends the contract.
- The Pause menu and Field Guide suspend production, franchise timers, and
  tracked playtime. The atlas and business counters keep production running.
  F5 saves from the atlas and shows the result there.
- Wrecks stop and release the player to a clear exit when one is available.
  Being downed at the wheel stops the car. Interior wallet labels leave
  space for large amounts, and storefront switches reset on career changes.
  Angle normalization stays constant-time as the session clock advances.
- Save revision 7 and the revision 2–6 readers carry forward. Active jobs,
  traffic, and pursuits remain session state.

The regression suite and sanitizer soak exercise gameplay, save migration,
profiles, businesses, and the Windows entry point. The input-driven playtest
completed all eight jobs and visited all 78 interiors, operating all 63
ordinary storefront fixtures and returning to the street through their doors.
See [the QA report](docs/QA-REPORT.txt) for the earlier replay details.

## Current player experience

- A new file begins in the editor, then Apartment 4, with no gun, car or
  money. Follow the first-shift GPS to Mercury Dispatch, interact with Mara
  and visit the courier counters. The introductory courier trip is untimed.
- Legal jobs pay clean cash and street contracts pay dirty money. Follow
  the gold vehicle marker or the named GPS destination. Race gates score
  on crossing. Press E at a counter, cargo, console or stationary handoff
  according to the objective.
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
- UI keeps clean cash green, dirty money red, including sale notices. Do
  not collapse them into one value. Purchases that draw heavily on street
  cash can trigger Revenue, seizure and a case. Clear the case through
  restitution or a fine; no “wait in prison” loop.
- Legal franchise buy-ins: Mercury Dispatch $360, Ferry Market $710,
  Wrecker Yard $1,200. Passive clean income per 120 active seconds:
  $24/$36/$49. Purchase via Business Office and enter the staff door.
  The fictional desk converts a limited amount of dirty money with a 25% fee,
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
| Shared engine | `src/game.c`, `src/game.h`: draw primitives, fixed 60 Hz tick, world, wallet, save schema, dispatch hooks and scene routing |
| Contracts | `jobs.inc`, taxi logic in `game.c`, interactions in `interiors.inc` and `streetside_interiors.inc`: staged vehicles, job objectives, navigation, handoffs and payout |
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

The Sidewalk Patrols keyboard tour starts a new career, finishes the eight
contracts, uses the counters and doors in all 78 buildings, buys the Mercury
franchise and collects a settlement. Run `bash scripts/test.sh --tour` to replay
its 57,765 fixed steps through the Windows executable using simulated Win32.
The fixture stores a final save digest and run-length encoded keyboard input.

The extended sanitizer run passed 1,200,000 ticks and 14,496 renders across
48 starting contexts. Run `bash scripts/test.sh --extended` to repeat it.
The follow-on economy playthrough covered all five garage cars, all three
franchises, both tier-three facilities, the three stock sales, Revenue and
Breakwater. See the [QA report](docs/QA-REPORT.txt) for scope and results.

`bash scripts/test.sh` checks new career, portraits/presets, save migrations,
deletion and failure behavior, gameplay through all doors, business and wallet
state, and the exact generated PE via simulated Win32. Use `--soak` for the
180,000-tick sanitized stress run. The Port Vanta Contracts input replay
completed all eight jobs and the Mercury franchise settlement in 29,053
frames through the packaged Windows executable under simulated Win32 on
Linux. The profile tests covered delete confirmation, surviving file
load, fresh creation, 360 distinct previews and letterboxed map controls.
Physical Windows 11 input and display testing remains outstanding.

Known gaps, not promises: active jobs and pursuits reset on relaunch or slot
change; progress such as wallet, case, property, stock, vehicle condition and
customization is persistent. Pre-update career time is unavailable. There is
no audio, multiplayer, installer or cloud save sync. Windows verification
should include a real resizable-window/HiDPI session, first courier and taxi,
an indoor menu, clean cash and dirty money notices and an intentional delete of a disposable
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
