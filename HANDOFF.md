# Dead Channel | Project handoff

## Current release: Conversation Hold Hotfix

Downloads: [Windows 11 x64](downloads/Dead-Channel-Conversation-Hold-Hotfix-Windows-11-x64.zip) ·
[Linux x86-64](downloads/Dead-Channel-Conversation-Hold-Hotfix-Linux-x64.tar.gz) ·
[SHA-256 checksums](downloads/SHA256SUMS.txt).

These files are the exact tested archives delivered in chat. Each contains
the runnable game, rebuildable source, and player guides. Intermediate chat
archives are not part of this repository update; previous published downloads
remain available. This is cumulative from the last published Port Vanta
Sessions release, with Vault 39 After Hours, the weapon wheel and med-patch
utility, Windows display fix, pistol pose and depth fixes, NPC encounters,
and the Field Guide and Conversation Hold hotfixes. No venue was added
or removed in this latest hotfix.


Based on exact delivered Guide Hotfix commit 11c25ee. The NPC choice
panel pins one identity and pose. The selected pedestrian, patrol,
rival, rider or crossing worker pauses their own path and faces the
player without altering their resume heading. Procedural Vault and
greenhouse actors freeze their rendered pose and rejoin their schedule
when the choice closes. Their unrelated venue timers continue; only
the selected person's hands and movement pause. Other actors remain
independent.

Esc, Say Hello, and Say Nothing release the hold on that input tick.
Rob passes the same pinned identity/pose to the existing covered draw
and fight transaction. This changes no bindings, payouts, production,
save fields or venues. See docs/QA-REPORT.txt for native launch evidence.

## Port Vanta Encounters Guide Hotfix (local delivery)

The added Shift+E entry made the nine-row street column touch the legend,
and longer labels exceeded its width. Keep the eight-or-fewer-row
column spacing and use a compact row step for nine rows, with short
labels. All rows now pass fit checks above the unchanged guide legend.
Only the guide, release title and tests changed; the NPC encounter code,
controls, economy and save bytes didn't. See docs/QA-REPORT.txt.

## Port Vanta Encounters (local delivery)

Based on exact Pistol Depth Fix commit a937a34. No withdrawn venues
were added. Visible physical NPCs have stable session identities in
`npc_encounter.inc`; video/photo preview sprites aren't world actors.
`npc_greetings.inc` contains the authored archetype pools and produces
a nonrepeating 50-line deck per identity. The dialogue doesn't pause
traffic or production. Normal E service/door/quest priority is retained;
counter menus and Shift+E provide explicit person interaction at shared
hotspots.

An explicit Rob selection equips the owned pistol without firing.
Its separate bounded RNG fixes the 35% branch and uniform $15-$155
amount once per target per session. Surrender credits dirty cash once.
Refusal uses the same weapon/punch range, cooldown and hit geometry,
with room or street collision checked from the body and muzzle. Regular
pedestrians, guards and officers follow their existing down/respawn
behavior; protected contacts/passengers reappear with native duties intact.
Fight, greeting and cooldown data are not serialized; the original wallet
and Rev2 through Rev11 migration remain unchanged. See docs/QA-REPORT.txt.
No remote publication.

## Pistol Depth Fix (local delivery)

Based on the exact Compact Pistol Stance commit 738155a. No venue or
story changes. Aiming straight up previously placed the barrel on the
head's centerline and painted it over the hair. `pulse_pose.inc` now
puts the near-north grip beside the shoulder, `pulse_sprite.inc` keeps
the hands compact, and `figure_held` paints upward aim behind the head.

The muzzle remains the shared world firing point even during recoil
and point-blank tuck. There is no added forward range or through-cover
exception. Keep both the original forward and offset muzzle collision
checks. Wall carry, wheel, presentation and both save encodings remain
unchanged. See docs/QA-REPORT.txt for verified files. No GitHub push.

## Compact Pistol Stance (local delivery)

Based on Pulse Pistol Pose. The old rig reached nearly the player's full
height and used a fixed upward slide highlight, which read as a narrow
line on a north/south barrel. `pulse_pose.inc` shortens the shared reach;
`model_person.inc` drops the upper arms near the ribs and fills the bent
sleeves. `pulse_sprite.inc` draws a colored barrel and outline across a
perpendicular screen-space axis so the volume survives diagonal turns.

A separate single-hand side carry replaces the solid-cover muzzle clamp.
Street collision queries and both interior colliders feed a five-clear-
tick latch. Side selection favors open space, the barrel points down,
and the hands and barrel sway with gait. The release rise avoids contact
flicker. The pistol won't fire or start a heat/cooldown event in carry.
Point-blank actors still use the existing shot cone; obstacles beyond
the raised weapon still stop the ray at the first obstruction.

Keep the muzzle pixel, stored firing anchor, flash and trace together
if editing this pose. The movement inputs, range, cooldown, damage,
cover guards, wheel, display fix and save format did not change. See
docs/QA-REPORT.txt for actual launcher and raster proof. No remote
publication.

## Pulse Pistol Pose (local delivery)

Based on Med Patch Utility. The outfit rig now blends a held pistol
with step and aim, keeps the item visible when equipped on foot, and
hides it after selecting fists or entering a car. Indoor pose is visual
only. The street trace and flash use a shared world muzzle, checked
from the feet and again along the shot so it cannot bypass cover.

Range, cooldown, damage, and controls are unchanged. The synthetic
302-frame keyboard replay passed all 16 corners of the firing pose test
with matching bytes across both platform entry points. See docs/QA-REPORT.txt
for tested builds and limits. No remote publication.

## Med Patch Utility (local delivery)

Based on Display Hotfix. The reference's parachute position is the
detached lower-right accessory at 4:30, not the inner bottom-left wedge.
The med badge now matches that position relative to the ring. It shows
the existing count and V action without affecting weapon hover. The
old medic sector is neutral. Both keyboard/mouse hosts passed their
stocked-saves replay and the Windows hotfix still clears only margins.
Save layout, healing, price, and world behavior are unchanged.
Verification and executable hashes are in docs/QA-REPORT.txt.
No remote publication.

## Display Hotfix (local delivery)

Based on Weapon Wheel. The Windows host now confines GDI black fills to
the bars outside the logical viewport and coalesces pointer updates into
the next normal frame. The old Wheel build repainted the entire client
black ahead of each pointer-driven letterboxed blit; a 75-frame exact PE
test reproduced 34 such fills. The new build made no black fill touching
the game view, still drew seven wheel frames and wrote the same save.

The native game, wheel selection, background dim, save format and SDL
rendering are unchanged; Linux gets only the cumulative window title.
Existing careers and any stored patches survive the update. See
docs/QA-REPORT.txt for binary and test details. No remote publication.

## Weapon Wheel (local delivery)

Based on Vault 39 After Hours. Hold Tab to choose an owned weapon by mouse proximity; release to equip. The wheel also shows stocked med patches and accepts V for one patch per press. Old saves load without losing career data. The actual in-game equipment changes only on Tab release; focus loss
cancels the preview. The wheel consumes input and holds the simulation
without opening over another menu. The radial art is drawn by the engine
and does not embed the reference image.

Ferry Market's $25 purchase stocks a patch at full HP and still treats an
injury immediately, up to 50 HP. The game saves 1–99 patches using the
formerly reserved byte at offset 267 only under Rev11. History through
Rev10 still loads; an empty inventory still writes original Rev10. An older
executable cannot read a presently stocked Rev11 career. Weapon preference
is session-only; on load, the pistol is the default if owned.

Both exact executable hashes and native input routes are in
docs/QA-REPORT.txt. No physical Windows test or remote publication.

## Vault 39 After Hours (local delivery)

Built from the exact Port Vanta Sessions tree published as dc3f2e6f6ca6ac50586c95994e76afce0f69fecb. Vault 39 gameplay is isolated to its named interior. The nightclub reuses gameplay collision footprints with new procedurally
drawn clublights, DJ equipment, bar furniture and hideout dressing. Mika's
small deterministic serving loop is independent of street actors and funds.
The staff door leads to an isolated layout in the same venue. Ivo listens,
offers the four existing contracts and accepts the old lockbox handoff only
backstage and when pursuit has cleared. See docs/START-HERE.txt.
The approved shared procedural art pass in src/model_person.inc and
src/model_vehicle.inc updates live people and cars without new asset
downloads, saved look fields or vehicle behavior changes. No remote
publication.

- Builds: Dead-Channel-Vault-39-After-Hours-Windows-11-x64.zip and
  Dead-Channel-Vault-39-After-Hours-Linux-x64.tar.gz; checksums under downloads.
- Exact unmodified executables both replayed the 3,351-keyboard-frame Vault
  Run and matched all 12 staged captures and 304 save bytes (11fd06d2).
  Confirmed continuing compatibility with the 3,664-frame opening
  (a6727845). See docs/QA-REPORT.txt for build hashes and scope. The Windows
  entry was simulated; no claim of a hardware Windows run.


## Current release: Port Vanta Sessions

Downloads: [Windows 11 x64](downloads/Dead-Channel-Port-Vanta-Sessions-Windows-11-x64.zip) ·
[Linux x86-64](downloads/Dead-Channel-Port-Vanta-Sessions-Linux-x64.tar.gz) ·
[SHA-256 checksums](downloads/SHA256SUMS.txt).

The public download archives are the same tested packages distributed in
chat. Each includes the runnable executable, rebuildable source and player
docs. The top-level README and handoff add the repository download links.

Cumulative update based on Easy Entry Hotfix. The new city listings and their world consequences are session-only. Ordinary jobs, all save slots and the 304-byte Rev10 layout remain compatible. Verification details below and in docs/QA-REPORT.txt.

### Release scope and verification
- F6 / Pause > City Listings exposes 17 playable entries across all named
  A/B groups. Session goals and personal pins are tracked separately.
  The original first shift, regular jobs, banking and Revenue remain intact.
- src/verdict_api.inc defines events, mission ownership and goal semantics.
  Group fragments compile into the shared native game. A1 generates original
  audio; Linux queues PCM via SDL2 and Windows dynamically opens system
  WinMM. Audio is optional. No recordings, new tram line or bundled DLL.
- Session souvenirs, footage, posters and ghost laps reset on actual
  career activation/relaunch. Already-earned wallet outcomes remain saved;
  Rev2-9 migrations and the 304-byte Rev10 layout have not changed.
- docs/CITY-LISTINGS.txt is the full player guide. docs/QA-REPORT.txt
  lists tested binary hashes, exercise-by-exercise results, and limits.
  The downloads manifest identifies archive checksums outside the embedded
  source (to avoid self-referential hashes). Earlier downloads and manifest
  entries are retained.

## Easy Entry Hotfix: Windows + Linux

This cumulative hotfix includes the three verified fixes from the ongoing
40-hour playtest. Both launchers read `Dead Channel | Easy Entry Hotfix`.
No GitHub update is part of this chat delivery.

1. **Field guide:** F1 dims the room or atlas you actually opened it from.
   The previous view remains visible, clocks/route state remain paused,
   and returning restores that room/map. It is not painted over an
   unrelated street scene.
2. **Map controls:** clicks in the black letterbox margin do not set or
   clear atlas GPS pins. A real right-click inside the framebuffer still
   clears the pin. Shared logical bounds protect both launchers.
3. **Vehicle staging:** taxi and sprint loaners and garage cars prefer a
   clear curb outside the immediate E-door reach. E still prioritizes
   job steps, doors and ATMs. The previous staging search is the fallback
   when every preferred spot is unavailable.

Both platform launchers share the same economy, vehicle handling and
304-byte Rev10 saves with Rev2–9 migration. Manual Windows/Linux transfer
remains safe after closing both copies and backing up any target slots.
Linux continues to require x86-64 glibc 2.34+ and SDL2 runtime.

The hotfix `--tour` tape is `scripts/qa/curb-access-tour.txt.gz`. The
3,664-frame run approaches the car directly, completes the courier and
taxi shifts with $318 clean cash, 100 HP and one fare, and hits explicit
save digest `a6727845` on both launchers. Keep the original Easy Entry
input tape as historical; do not change its digest to fake a pass.
See the current QA report for details and limits. The release explicitly
does not include the later ATM candidate.

Verified executable hashes (archive hashes ship in the separate manifest):
- Windows x64 PE SHA-256: `93bb63de65c438e076534b3ac3a09e68fc2b7579f0791f94e71af480b2187e50`.
- Linux x86-64 ELF SHA-256: `acdef8ed101e2c33c070263fc91518adf7f35aad2b60b7876b969e94aab9acdf`.

## Previous Easy Entry: Windows + Linux build

The game now has two native x86-64 launchers. `src/linux.c` presents the
same 960×540 CPU framebuffer through SDL2 with black letterboxing, maps
window mouse coordinates through the actual drawable size, routes
keyboard/ASCII text, and pauses simulation and clears held inputs when
unfocused. A key repeat after focus regain cannot generate a new
interaction. `src/windows.c` maintains the existing native PE entry.
No shared gameplay rules or title changed; both windows read
`Dead Channel | Easy Entry`.

- `bash scripts/build-linux.sh` builds `build/DeadChannel`; the tested ELF
  requires x86-64 glibc 2.34+ and `libSDL2-2.0.so.0`. The SDL2 SDK is
  optional for builds: `src/sdl2_linux.h` offers an x86-64 SDL2 ABI
  fallback when SDK headers/pkg-config are unavailable. Neither platform
  needs Wine, a browser, an account, or a network at runtime.
- `src/linux_save.c` uses the private user-data folder
  `$XDG_DATA_HOME/DeadChannel` (absolute XDG paths only), otherwise
  `~/.local/share/DeadChannel`. It shares exact slot names and the Rev10
  layout / Rev2–9 migrations with Windows. Read must match exact file size.
  Stage files are exclusive and private; writes flush before commit.
  Rename replaces existing saves; an atomic hard link commits a new slot
  without replacing another copy's save. Keep the target filesystem's
  POSIX link/rename semantics. Existing-slot simultaneous writes are
  still last-successful-write; backups and transfers are manual.
- Use `bash scripts/test-linux.sh` for the exact ELF event/render/save
  check under SDL dummy video and POSIX disk tests. The current
  `bash scripts/test.sh --tour` checks its existing Windows PE.
  See `docs/QA-REPORT.txt` for the verified results and exclusions.
- These local Windows/Linux archives have not been pushed. Before
  publishing another build, update this handoff for both targets,
  record both archive digests, verify matching binaries, and keep
  the exact save compatibility and test environment explicit.
- Linux binary SHA-256: `335e00c965f935970a283a4f2eae54a175bcbd20c26d307e6e77825b162d12f0`.
  Windows binary SHA-256 remains `d22b3cf14e7b5a0a106d665f4e3e06e44217776c96abcc72daa108a5c018900b`.
  Archive hashes accompany the distributed files separately.

**Previously published Windows release** — [Easy Entry for Windows 11 x64](downloads/Dead-Channel-Easy-Entry-Windows-11-x64.zip), November 26, 2026.
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
