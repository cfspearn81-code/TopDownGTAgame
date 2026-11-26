# Dead Channel | Project handoff

**Current release** — [Easy Entry for Windows 11 x64](downloads/Dead-Channel-Easy-Entry-Windows-11-x64.zip), November 26, 2026.
Cumulative Lantern Fix build, based on City Rhythm
(`98b10f29a03c0b910be4e696ae76fdacdceeb746`). Earlier download files and
their checksums are retained.

- ZIP SHA-256: `7547002a48b9d0b75edec2684de3ebba7ecfddf458ca9ff9d1dd627eae5b6cad`.
- Embedded and reproducibly rebuilt `DeadChannel.exe` SHA-256:
  `d22b3cf14e7b5a0a106d665f4e3e06e44217776c96abcc72daa108a5c018900b`.
- Launch: extract the ZIP and run `DeadChannel.exe` (title `Dead Channel | Easy Entry`).

- E now boards or steals a stopped or slow car and still exits it.
  Job actions, doors, and ATMs retain priority when nearby. Holding E
  after boarding does not immediately exit; release it to interact again.
- Enter still confirms menu choices and boards trams, but no longer
  boards street vehicles. No save-format, economy or handling changes.
- Current `--tour` fixture: `scripts/qa/easy-entry-tour.txt.gz`. The
  prior Lantern Fix recording is unchanged for the earlier executable.

**Previous prepared Windows build** — Midnight Clock: Lantern Fix, November 26, 2026.
Cumulative Windows build based on Speed Tune. Not pushed to GitHub.

- The Lantern operator's work targets come from the same active-pot
  layout as the plants. Empty and locked pots are no longer watered or
  otherwise tended as part of an active batch.
- Lantern no longer offers a distributor. Existing Lantern distributors
  become operators, without a payout or loss of funds or stock. An
  in-flight batch still completes into storage, allowing a one-time
  overflow beyond 60 units; no new batches start until there is room.
  Cinder still offers its own distributor.
- The player and NPCs are a little smaller in live play. Foot travel
  is 20% slower for the player and moving human NPCs. Camera framing,
  vehicle handling and career portraits are unchanged. A pedestrian
  already retreating from a crosswalk can move out of a turning car's
  collision margin, but never deeper into it.
- The current `--tour` fixture is `scripts/qa/lantern-fix-tour.txt.gz`.
  The fixed-frame older tours remain historical and do not match the
  slower on-foot pace.
- Retains the Ivo fix, 24-minute clock, operator, packing, stock shelf
  and Rev10 career saves. Other economy rules are unchanged.

**Previous prepared Windows build** — Midnight Clock: Speed Tune, November 26, 2026.
Based on the delivered Midnight Clock game and keeps its clock, Ivo
placement, Lantern operator and save compatibility. Local Windows ZIP.

- Each healthy player-driven car tops out at its own former 10-condition
  forward speed. All five cars remain faster than walking; no guarantee
  that all beat sprinting. Damaged cars still lose speed. Throttle
  response, reverse, steering and braking rules are unchanged.
- Current `--tour` fixture is `scripts/qa/speed-tune-tour.txt.gz`; the
  original Midnight Clock replay is retained as a historical recording.
- No new save format; saves made in Midnight Clock still work.

**Previous prepared Windows build** — Midnight Clock, November 26, 2026.
Based on the delivered Revenue Deadline source. Local Windows ZIP; not
published to GitHub.

- The upper-left clock uses 24-hour time. One active real minute is
  one game hour; one real second is one game minute. Midnight lands
  after exactly 24 active minutes, and resets the daily dirty-spend
  allowance. The separate five-minute case seizure timer is unchanged.
- Ivo Slate in Vault 39 stands clear of the lounge furniture, with
  the contract interaction still accessible.
- Hiring Lantern's operator adds a working character in the back room.
  The scene follows the existing batch stages, from planting to loading
  boxes. The visible stack reads the current Lantern stock across both
  products; the room does not change production, sales or storage rules.
  The normal 60-unit capacity fits on one shelf (zero to three cartons).
  The shelf label still shows the count for older saves above capacity.
  The worker's motion is visual only and does not block the player.
- Save Rev10 keeps the same 304-byte layout; Rev2 through Rev9 still
  load. An old career retains its elapsed time, dirty spending, and
  active case countdown. Pending franchise deposits retain their
  next-or-following closing under the new clock. New Rev10 saves
  require this release or newer.

**Previous prepared Windows build** — Revenue Deadline, November 26, 2026.
Based on the delivered Revenue & Banking source. This ZIP has not been
pushed to GitHub.

- The first strict crossing of $500 dirty spending still triggers the
  previous dirty-only seizure and opens a Revenue case. From that point,
  an open case seizes both clean and dirty cash carried on the player
  every five minutes of active gameplay. No funds are taken from the
  shared ATM account. An empty wallet does not stop the clock.
- Paying the fine or finishing all three unpaid service shifts clears
  the case and ends the countdown. Pausing or closing the game doesn't
  make the timer run offline. Save/reload resumes the same countdown.
  Older careers with an open case start with five minutes remaining.
- The street HUD and Revenue Annex show time remaining. Repeat alerts
  show separate clean/dirty amounts when a wallet is actually seized.
- Save Rev9 (304 bytes) keeps all Rev8 content in place, with int32
  remaining simulation ticks at offset 296 and FNV checksum at 300.
  Rev2 through Rev8 remain readable. Bank balance stays at offset 288.

**Previous prepared Windows build** — Revenue & Banking, November 26, 2026.
Based on the delivered Fast Lane source. No remote publication.

- Dirty spending accumulates only from the dirty-money portion of successful
  checkouts. The existing game day lasts eight active minutes. Exactly
  $500 is safe; its first strict crossing in a day completes the purchase,
  removes remaining dirty funds and unclaimed dirty-origin settlements,
  and opens a case. Clean cash and personal bank balance are not seized.
  Clean-only spending, refused transactions, transfers and holding money
  don't count. The day meter clears at the same boundary as franchise closing.
- Three sidewalk ATMs in Tide Market, East Freight, and Lantern Ward share
  one protected clean-cash account. The atlas can pin the mint $ markers.
  Walk up and press E. Use up/down for direction, left/right for amount,
  Enter/E to confirm, Esc to close. Transfers save immediately and roll
  back if the write fails. Spending and fines still use the on-hand wallet.
- A true seizure gets a centered, timed ASSETS SEIZED card, an amber
  warning symbol, a brief damped shake, and directions to Revenue Annex.
  Low FX disables shaking. Gameplay is not blocked by the alert.
- Save revision 8 adds bank and daily spending to the Rev7 layout.
  Older saves Rev2–7 migrate. Confirmed cases survive; old pending audit
  warnings from the retired rule do not confiscate funds after migrating.
  Revenue continues to accept the fine or three unpaid qualifying shifts.

**Previous prepared Windows build** — Fast Lane, November 26, 2026. Based on the
delivered Inside Port Vanta source; the custom buildings and existing career
files are preserved. This Windows ZIP has not been pushed to GitHub.

- Swifter forward acceleration and increased top speeds make every healthy
  car faster than walking or sprinting. Low-speed steering, deliberate
  reverse limits, braking, and camera lead remain controllable; the higher
  speed is checked in shorter collision steps. Pedestrian movement and
  ambient traffic speeds are unchanged.
- Revenue service is now unpaid. Three completed courier/recovery service
  assignments clear the hold. The third still pays $0. Later regular
  shifts pay again, while unrelated work keeps its normal rules. No new
  save fields; existing case and career progress remains intact.
- The long Inside Port Vanta keyboard recording is historical because
  the same held driving inputs now travel farther. Test coverage for the
  new handling and service rules is documented in `docs/QA-REPORT.txt`.

**Previous prepared Windows build** — Inside Port Vanta, November 26, 2026.
Based on the delivered City Polish source (upstream City Rhythm merge
`98b10f29a03c0b910be4e696ae76fdacdceeb746`). This handoff accompanies
the local Windows ZIP; no new GitHub revision was published.

- 15 named venues and four rear rooms now use distinct themed wall and
  floor staging in `src/named_sets.inc`: sorting lanes, car display
  platforms, clinic bays, small apartment furniture, precinct offices
  and cells, and specialized workspaces and staff floors. Public and
  staff counter coordinates and walkable footprints remain compatible.
- All 63 street addresses get architecture and props from a combination
  of theme and address. `storefront_setdress.inc` handles the themed
  scenery; `storefront_addresses.inc` handles building-specific traits
  and visitor plaques. The tram halls have actual visible track and a
  platform, while the timetable and ride retain their original controls.
- Rebuilt from City Polish, preserving its gameplay changes, save
  isolation, controls, and Rev7 reader/writer (legacy Rev2–6 supported).
  No new persistent fields or new mandatory interactions.
- `bash scripts/test.sh --tour` now also runs the indoor route regression.
  Replay before release. Other visual proof commands and the documented
  verification limit are in `docs/QA-REPORT.txt`.

**Previous prepared Windows build** — City Polish, November 26, 2026. Based on
upstream City Rhythm merge `98b10f29a03c0b910be4e696ae76fdacdceeb746`.
The player ZIP accompanies this handoff; its source is not yet published
as a new repository revision.

- Shared rotated-vehicle checks make parked/wrecked vehicles block hits,
  melee reach and car boarding; weapon tracers end at impact. Restored
  players find vehicle- and scenery-clear footing. Brake/reverse lights
  and bounded, temporary skid marks add driving feedback without changing
  the steering input model.
- An active Revenue audit blocks all dirty-funded checkout but not
  otherwise allowed clean purchases. The first paycheck marks Nightglass.
  A taxi/sprint car wreck before starting can be recovered with a drivable
  licensed car or ends the contract clearly. Repeat taxi fares remember
  and guide back to the correct car.
- The HUD uses rendered text widths for wrapping, headings fit by the
  wallet, help uses two columns, and radar shows district or turn preview.
- Save writes use unique exclusively created stages in the same directory,
  leaving old data alone on failures and keeping no-replace new-save
  behavior. New slots retry only if another copy claims their slot. Two
  copies playing the same existing career still use last-successful-write.
- Save revision 7 is unchanged; revisions 2–6 still load. The tour's
  input is unchanged, with two additional HP at the end because cars now
  block attacks. QA details are in `docs/QA-REPORT.txt`. A physical Windows
  11 playthrough is still outstanding.

**Previous upstream build** — [City Rhythm for Windows x64](downloads/Dead-Channel-City-Rhythm-Windows-11-x64.zip), November 26, 2026.

City collision now uses a common set of per-block obstacles for people, car
bodies, sight lines and routing. Building bounds cover the visible north roof
edge and projected east wall. Park paths and storefront approaches stay open.
The 114 rendered trees have solid trunks. Street sprites draw in depth order,
and the facade covers an actor passing behind it. Garage models have distinct
cabin trim, including a ribbed cargo van and a striped Nightjar. The rotated
windows, character shadows, storefront signs and fixture captions were revised.

Mara's first shift pays for a Mote. Later manifests rotate destinations.
Recovery, courier, Vault Run, Signal Raid and Rival Collection are untimed.
Taxi fares display the current tip, and passengers accept curbside stops.
The Glass Mile sprint reports a finish time and session best. Holding E at
speed brakes and exits. Releasing E cancels the pending exit. On foot, Enter
boards cars. E is reserved for doors and other world interactions.

The atlas supports all 78 doors and shows owned-business status. A newly
accepted job selects its GPS route. At either tram hall's timetable, E shows
the other station and Enter buys a $5 clean-cash ride when the player is free
of an active contract or pursuit. The car stays where the player parked it.

The save layout remains revision 7 with the existing revision 2–6 readers.
Loading a career whose position now intersects scenery finds nearby clear
ground. Sprint records last for the active career session. The verified archive
checksum is in [SHA256SUMS.txt](downloads/SHA256SUMS.txt). Physical Windows
11 input and display verification remains open.

**Previous build** — [Sidewalk Patrols for Windows x64](downloads/Dead-Channel-Sidewalk-Patrols-Windows-11-x64.zip), November 26, 2026.
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

## How the current game plays

**Start and jobs.** A new save begins in the character creator, then
Apartment 4 with $0, fists and no car. The GPS leads to Mercury Dispatch.
The first courier shift introduces indoor counters and pays $140, enough
for the $125 Mote at Nightglass Motors. All eight jobs are live:

| Contact | Job and completion |
|---|---|
| Mara, Mercury Dispatch | Courier: deliver two parcels at indoor counters and return the signed manifest. Later routes rotate. Taxi: stop close to a real curbside rider, let them board, drive them to the chosen destination and wait until they exit. Base fare is locked to route distance; crashes and rough driving affect the tip. After the first completion, T accepts another randomized call without returning to Mercury. |
| Dax, Wrecker Yard | Licensed recovery: restart the actual stalled van with E, board it with E, then stop at Dax's bay and press E for handoff. |
| Paz, Glass Mile | Timed sprint: use the same licensed car from the start to the finish. Gates advance automatically on crossing and the session remembers the best finish. |
| Ivo / Vera | Vault Run: collect a lockbox from its counter and lose pursuit before returning it. Hot Property: steal the marked Vectra and stop at the handoff once clear. Relay and Rival contracts: interact at the marked relay rooms or claim the guarded consignment and return it to Vera. |

**City and vehicles.** All 78 physical buildings are enterable: 15 named venues
and 63 ordinary storefronts with 19 archetypes and address-specific details.
Four gated rear rooms add workplace interiors. Eight retired building-array
entries are not physical addresses. The atlas pins doors, job targets, tram
halls and three ATMs; the GPS follows remaining streets and the driving radar
previews the next turn. Optical lines, physical footprints, roofs, doors,
sidewalks and collision masks must stay consistent.

Vehicles can be acquired, stolen, damaged and repaired. Solid chassis
block walking, boarding through wrecks and attacks; brake/reverse lamps
and brief skid marks give feedback. Standing at a door or ATM wins over
a nearby car. Press E next to the car, away from the competing interaction.
Each healthy car retains its former 10-condition forward cap (Mote 151.8,
Vectra 180.4, Pallas 165, Nightjar 205.15, Courier 4 157.3 world units/s).
Damage still lowers the cap. Walking/sprinting are 98.4/146.4 units/s outside;
both are 20% below the old pace. Live bodies are roughly 11% smaller; previews,
facing arrows and camera framing retain their behavior. Officers use sidewalks
and crosswalks, with eight fixed posts/units. Keep actual traffic routes and
the retreat-only clearance check intact.

**Wallets, banking and Revenue.** Clean cash is green, dirty money red.
Clean-only banking via the three ATMs is shared and protected. Spending exactly
$500 in dirty money in one game day is safe; the first checkout that crosses
$500 completes, seizes remaining dirty money and unclaimed dirty-origin
settlements, and opens a case. While a case is open, Revenue seizes both
on-person wallets every five active minutes, not the bank. Pay the $180 fine
or complete three unpaid courier/recovery service shifts. The final service
shift still pays $0; later shifts pay normally. There is no prison waiting
loop. The 24-hour clock equates one active real second to one game minute;
midnight after 24 active minutes resets the daily spend. Case and day clocks
are separate. Pausing/help and closing the game don't advance active time.

**Businesses.** Legal franchise prices at Mercury, Ferry Market and Wrecker
Yard are $360/$710/$1,200. They earn $24/$36/$49 clean per 120 active seconds.
A separate fictional exchange at the same owner's rear desk takes a 25% game
fee; a ready settlement requires manual collection at that very desk.
Deposits in the final 45 seconds defer to the following closing. Lantern
Grow Loft ($3,000) and Cinder Works ($10,000) use abstract inputs, three
tiers and 120-second production. Lantern's hired operator works only occupied
pots, places cartons into visible stock, and no longer offers a distributor.
Old Lantern distributors migrate to operators; an in-flight harvest can
temporarily take stock above the normal 60-unit limit and the usual capacity
gate applies before starting the next batch. Cinder's distributor remains.
Keep these fictional systems abstract, not real-world recipes or processes.

## Input and Career Files

`WASD` / arrows move and steer; `Shift` sprints or handbrakes.
`E` interacts, boards or steals slow vehicles, and exits them; holding
E at speed brakes to exit. The same held E that boarded a car does not
exit it until released and pressed again. Actual job actions, doors and
ATMs take priority over a nearby vehicle. `Enter` confirms menu choices,
uses atlas pins or boards a tram, and does not board cars. `X` punches;
`Space` attacks with fists or equipped weapon. `T` accepts another unlocked
taxi fare; `M` opens the atlas (left click pin, right click clear).
`F1` field guide, `F2` low FX, `F3` Career Files, `F4` new career,
`F5` save, `Esc` pause/back.

Each career card shows its portrait, name, playtime and both wallet balances.
The character creator has ten categories, 360 editable starter looks, shuffle
and a four-way preview. The upright career preview intentionally differs from
the world projection. Do not change world collision/camera to fix a preview.
The selector guards the active slot from deletion; use a fresh launch to
delete it deliberately. The confirmation names the exact slot and defaults to
cancel. Failed deletes must not remove other slots or change the write target.

## Source map and implementation invariants

| Area | Files / patterns |
|---|---|
| Engine / input | `src/game.c`, `src/game.h`: software rendering, shared 60 Hz tick, save schema, wallet and production, scene routing, common `street_use_target` and `vehicle_use` for HUD / E input |
| Contracts | `src/jobs.inc` and taxi state in `game.c`: call staging, role-based goals, swept sprint gates, stops, guard/handoff input and payouts |
| Native host and disk | `src/windows.c`: Win32 window, keyboard and WM_CHAR, letterbox mouse mapping, DIB, exclusive stage files and checked replacement; imports in `scripts/coff.py` and `scripts/check_pe.c` |
| City art and doors | `src/district_art.inc`, `src/navigation.inc`, `src/interiors.inc`, `src/named_sets.inc`, `src/streetside_interiors.inc`, `src/storefront_addresses.inc`, `src/storefront_setdress.inc`: exterior/roof masks, atlas, placements, accessible street interaction and indoor collision |
| Street traffic and pursuit | `src/street_life.inc`, `src/dispatch.inc`, `src/road_fx.inc`: rotated chassis, groups of pedestrians, dispatch state transitions, world-space tire marks |
| Careers and figures | `src/profiles.inc`, `src/character.inc` and figure renderer in `game.c`: active slot, deletion guard, name, saved looks, preview and live body scale |
| Economy and grow room | `src/legal_business.inc`, `src/bank_ui.inc`, `src/grow_room_art.inc` and `game.c`: checkout, game clocks and Revenue receipt, timed business states, animated actual pot targets |
| Builds and verification | `scripts/build.sh`, `scripts/coff.py`, `scripts/test.sh`, `scripts/check_pe.c`, `scripts/qa/` and `src/*test.c` |

The software framebuffer is 960×540 and the city grid is 2048×2048.
Compile `game.c` and `windows.c`, not the included `.inc` fragments.
The native host builds freestanding with a Win64 ABI from x86-64 Linux
using GCC, Python 3 and GNU binutils' `i386pep` emulation. Keep host
declarations, linked imports and the simulated test harness in sync.
No extra engine, asset directory, runtime bundle or online authentication
is required. The generated `font.inc` is committed; retain
[`docs/FONT-LICENSE.txt`](docs/FONT-LICENSE.txt).

**Save compatibility.** Rev10 is 304 bytes and reads Rev2–9. Saves live at
`%LOCALAPPDATA%/DeadChannel` as `career.dat`, historical `opening.dat`,
and `fresh-001.dat` through `fresh-250.dat`. New slots use no-replace;
existing slots use uniquely and exclusively created stages with atomic
replacement. A failed write cannot clobber the old file. Two processes
playing the same already-existing slot are still last-successful-write.
Do not use player saves in tests. Keep magic/revision, field ranges,
legacy layout and checksum validation in sync if touching save structure.
For useful Rev10 offsets, see the native boot QA scripts; no change to the
schema is needed for Easy Entry.

## Verification and remaining work

The exact Easy Entry executable and its current source passed
`bash scripts/test.sh --tour`. The Windows PE was exercised at its actual
entry point under simulated Win32 on Linux. The 3,252-frame keyboard tour
opens a career, completes the first courier shift, ignores Enter at the
staged taxi, boards with E, completes the fare and exits with E. It made
five atomic writes; the final Rev10 save has $289 clean cash, 100 HP,
two legal jobs and digest `16347d9b`. Tests also cover held E, no unintended
entry at speed, car theft and overlap priority; 256 indoor interaction
routes; legacy slot and economic migrations; and ATM, Revenue, midnight,
banking, deletion and window-size input. See [QA report](docs/QA-REPORT.txt).

The earlier 60,569-frame tour completed all eight jobs and toured every
interior. It is historical: `bash scripts/test.sh --historical-tour
/path/to/Inside-Port-Vanta/DeadChannel.exe` uses its earlier binary and
digest `3ac6dd6b` (City Rhythm's original binary instead ended
`f3397161` with the same actions). Fast Lane, Speed Tune and Lantern Fix
also have their own older fixtures. Do not “fix” a failed historical
fixture by replacing its digest; for changed controls or pacing, record
fresh real input and check final gameplay outcomes before locking the
digest. Existing `--soak` and `--extended` modes run 180,000 and
1,200,000 ticks respectively; the extended results in the QA history
are not a new run for this release.

**Still open:** physical Windows 11 launch, focus, key hold/release, window
resize and HiDPI. Use a disposable career for on-device testing. Active
jobs and pursuit reset after relaunch or slot changes; wallet/stock,
case/economy progression, customization and vehicle condition persist.
There is no audio, multiplayer, installer or cloud save sync. Pre-update
playtime was not recorded.

## Packaging checklist

1. **Before uploading any new build, the next AI or human maintainer must
   update this summary (`HANDOFF.md`) and the player docs.** Record the
   release name/date, actual changes, save compatibility, test status,
   outstanding problems and the matching archive/checksum. Do not publish
   a new build with a previous build's summary.
2. Rebuild using the published repository and run the tests. Build ZIP
   layout: `Dead-Channel/DeadChannel.exe`, player/QA notes, optional `Previews/`
   and rebuildable `Source/`. Exclude private metadata and real saves.
3. Run `unzip -tq` and compare the embedded EXE to the tested binary.
   Record `sha256sum` for the ZIP and update README's link.
4. Publish the matching source, handoff and artifact together; check that
   the GitHub path and checksum refer to the actual published file.

Current Windows archive and digest: [downloads/SHA256SUMS.txt](downloads/SHA256SUMS.txt).
