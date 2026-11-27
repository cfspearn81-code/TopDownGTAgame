# Dead Channel: Crossroads — Street Life stability update
**November 27, 2026 | Six parallel one-hour test lanes; one independent review**

**Verdict:** Install this update before taking another Afterimage booking if you keep graded stock in Private Quarters. The previous build could pay the correct show fee while silently erasing three inventory counts. This release separates the records. It also fixes real ways to get trapped in a wreck, lose earned income, or see a street encounter disappear simply by opening a menu.

## The save issue: what to do with an existing career

In a 2,274-input Copper Playhouse booking, the old game paid the promised $115 but changed the three Echo Bloom counts in the **Private Quarters graded shelf** (House C, Select B and Top Shelf A) from **11 / 7 / 13 to 0 / 0 / 0**. The corrected Windows and Linux builds paid the same fee and kept all 31 units. The satchel and the Signal/Lantern production stockroom use separate records and are not implicated. All three show destinations were driven to completion; both hosts produced identical saves on each route.

- **Normal saves:** no action required. New bookings store Bryn’s fee independently from graded stock.
- **Old ambiguous saves:** $115 in the old fee format is recognized automatically. The old $145 and $165 codes can also represent legitimate stock. Bryn lets you **keep the inventory, redeem the old fee, or back out without changing either**. Only the three affected bins are protected until you decide; the shelf’s map action points to Bryn. Ordinary stock elsewhere still works.
- **Already lost stock:** the patch cannot reconstruct counts the old game already erased. An intact save from before the loss is the only basis for restoring the original quantities.

## What actually ran

Each private tester used GPT‑6.2 Astra with Ultra reasoning. The clocks overlap. The game was exercised through input replays, staged engine scenarios, rendered frames, repeated stress suites and save comparisons, using disposable careers. These are **six measured hours of parallel automated coverage**, not six human playthroughs or six hours on the final build alone.

| Lane | Measured wall time | Representative coverage |
|---|---:|---|
| Street and police | 60m 01s active | 1,044 successful gameplay processes; randomized encounters, crowds, weapons, menu interruptions, pursuit and 41 separate paired launcher replay batches. |
| Driving and Motor Works | 61m 46s total; 60m 07s post-build workload | 832 seeded drives and 2.31 million input ticks. All 50 car/course/weather combinations finished a controlled traffic-free race; live traffic ran separately. |
| Jobs, taxi and navigation | 60m 12s | 566 passing test processes; all 17 listings checked for starting, browsing and cancellation, with selected endings played through. Each of the six new outbound taxi routes was actually steered on both hosts. |
| Buildings and venues | 60m 08s | 961 workloads, including 144 repeated interior tours across 78 interiors and five gated back areas; 37,296 route checks. All three show bookings driven through payment and reload. |
| Economy, banking and careers | 60m 24s | 84 workload rounds; 797 successful suite invocations. All five property routes tested on the released Windows build and both patched clients. Four intermediate compatibility/test-environment failures passed after correction or rerun. |
| Desktop and saves | 60m 15s rotating, plus an overlapping 60m 00s continuous native session | 221 suite invocations; 219 first-pass successes, two test-runner timeouts passed on rerun. A separate Linux game ran continuously for 3,600.006 seconds, presented 206,239 software frames and exited with a valid save. |

Linux ran natively with software SDL. The actual Windows executable ran under a simulated Win32 host, **not on a physical Windows PC**. Seeded tests sometimes set up hard-to-reach states directly (e.g. a full wallet or a boxed-in wreck); those are not counted as complete drives. The final integrated binaries matched the established 3,664-frame opening/courier/taxi career checksum, **a6727845**, and matched each other after a 30,003-frame driving tour. The final regression also exercised **1.2 million instrumented simulation ticks**. Those are additional checks, not extra wall-clock hours. Hardware FPS, physical audio output and every possible sandbox state remain unverified.

## Verified fixes, duplicates removed

**Street and police**
- Help, Signal and the save catalog no longer erase an active fight or dropped cash. The old game lost six out of six tested encounters/drops; the patch lost none and allowed only one pickup.
- Outer Loop pedestrians now respond to on-foot and vehicle contact under the same rules. Cover still blocks contact; people who are down cannot be targeted or struck repeatedly, and a vehicle impact applies its normal slowdown and dispatch consequence once.
- The finite police roster can physically reach all six outer districts and return to post instead of being confined downtown. Responders use roads, respect closures, do not silently acquire an unseen player and leave the player’s GPS alone.
- Regional pedestrians use six walkable shoulder paths instead of lane dividers. An independent traffic check found **0 road or obstacle incursions in 9,268 samples**.

**Driving and Motor Works**
- The chase camera now shows regional pedestrians and the waiting Afterimage performer; the performer no longer appears on the roadside after boarding. At night the headlights illuminate the road in chase view and stop at obstructions rather than shining through a barrier.
- Entering or leaving a car can no longer bypass a closed Freight Parkway barricade. A separate wreck-only fallback gives a surviving driver a reachable spot on foot when all ordinary doors are boxed in, without sending them through walls or closures.
- Race ghosts no longer jump sideways or turn 90° in one frame at gates. The same course timing is retained.
- Salvage GPS picks the nearer reachable parts intake instead of always sending drivers to Nightglass. A blocked exit or payment no longer consumes the final scan: clear space and retry the same stamp.

**Jobs and navigation**
- F6 now pages between newspaper issues correctly in both directions, including the last row and issue. Accepting a new fare opens a fresh shift receipt immediately instead of displaying the previous shift as ended.
- Hitting a pedestrian with a passenger aboard counts as an incident and forfeits the discretionary tip, not the promised base fare.
- GPS no longer doubles back immediately at the city/highway join: **104 of 4,320 sampled routes did so before, none after**. The Freight Parkway name no longer overlaps the minimap’s M key; 144 conflicting pixels became zero.

**Venues and careers**
- The Afterimage payment/inventory collision is fixed as described above. At Vault 39, Mika no longer takes orders or pours drinks while she is away from the bar in a confrontation. Drinks already on the counter remain collectible; service resumes when she returns.
- Bell Lane now identifies an owned stockroom rather than labeling it locked. Its interaction highlight agrees with the distance where E actually works.
- Sleeping eight city hours no longer creates active-play franchise wages or instantly triggers the five-active-minute Revenue sweep. The city clock, midnight reset, healing, crops, weather and timed settlements still advance appropriately.
- Saving and loading at seven HP now returns you to seven, not fifteen. At an ATM, moving left from MAX with $9 now selects $1 rather than silently leaving the transfer at $9.
- A Revenue case that takes nothing no longer announces “Assets Seized.” Actual seizure notices and the street ticker say what was lost, including canceled pending settlements. The bank stays protected.
- Imported or very long careers no longer risk an unloadable save when buying supplies at the unit limit. Full storage retains already-paid product, and capped job/sale/batch counters do not wrap. A maxed taxi counter now preserves Dispatch access after the next fare.

**Payouts at the wallet limit** — one family of problems, with distinct player outcomes:
- **Taxi and race:** completion messages report the amount actually credited, not the entire quoted fare or purse.
- **Motor Works:** salvage retains the vehicle when payment won’t fit and reports the actual credits awarded. A showcase retains its daily claim until it can be paid.
- **Shoal Walk:** the $75 conservation grant and $30 stipend become separate collectible receipts when necessary. Inez pays each once when there is room, including after a reload.
- **Businesses:** Kiosk 09 sells only whole units that can be paid for and previews units, proceeds and risk. Cinder holds a finished shipment until its full payment fits. An old exchange settlement preserves its unpaid balance instead of throwing the remainder away.

**Desktop, input and save protection**
- Windows F10 now responds to the system-key event Windows actually sends, restoring the export shortcut.
- Linux refuses to treat a short or different same-name migration backup as an intact copy; an exact matching backup can be reused. A failed-save **Keep playing** choice now survives the follow-on close event without disabling the next intentional quit.
- Linux rebuilds its framebuffer texture after a renderer reset and retries audio output when an unavailable or disconnected device returns. The radio need not stay silent until relaunch.

## GTA-style changes that are actually in the build

1. **Crosstown fares:** after you earn your first taxi license, Dispatch can send you from downtown to six regional drop-offs: Glass Bluff, Parkway Overlook, Belt Workshops, Harbor Pier, Salt Reach and Windmill Curve. Each paid once in the end-to-end drive.
2. **Passengers remember a threat:** drawing on your own waiting passenger cancels that booking. The old game let the driver take the passenger’s money, then collect a normal fare and full tip from that same person. Saying hello, failing to draw without a weapon or interacting with someone else does not cancel the booking.

The other entries above repair existing Street Life features; they are not being advertised again as newly added systems.

## Shortlist for a future update — not implemented

- Briefly outline the person E will engage when several pedestrians stand close together, without adding another permanent HUD element.
- Flash a short plus/minus time split at race gates; let regional riders occasionally call out a named landmark.
- Add a Signal **Owned businesses** page with each counter’s location, readiness and a GPS pin, not remote cash collection.
- Let completed show bookings trigger a short message from the artist with a pin to a future venue.
- Decide whether driving radio should continue or pause in the background, then make Windows and Linux agree. Also worth a targeted check: competing cover/talk prompts near Remi at the Afterimage. That prompt is not claimed as fixed.

## Combined release verification

The final builds matched the original 3,664-frame courier/taxi career
checksum `a6727845`. The combined host tests also covered the preserved
loft stock, active regional pursuit, pedestrian interactions, traffic
boarding, taxi quote and receipt, and frame diagnostics. A separate
sanitizer run passed 1,200,000 simulated gameplay ticks and 14,496 render
passes across 48 start contexts. Simulated ticks are not counted toward
any tester's actual hour.

Exact executable SHA-256, before packaging:
- Windows x64: `904f7d3f914f3f28c40e703131b1cb3dd551e8a52d8f52ecc19c2661e2e50424`
- Linux x64: `6f10a41a1a38a716b6f8f7104542f5bc60050a17eb7382038efda901fb721b96`

The repository download links point to the same verified archives that were
originally delivered directly. Historical builds remain in the downloads folder.
