# Dead Channel: Port Vanta Crossroads | Playtest verdict

**Six independent playtesters, one deduplicated list.** Baseline: [GitHub main at a545bd8](https://github.com/cfspearn81-code/TopDownGTAgame/commit/a545bd8fd6720bef690ee1410fb70ba97a2d15dd). The six passes produced **24 groups of reproduced and corrected issues**. A defect seen by multiple testers, or in multiple shops or on both platforms, appears only once. The results below describe the individually tested fixes. They do not imply a GitHub release.

**What you'll notice first:** Windows no longer quietly quits after a failed save. A side job can't skip the first courier mission. Taxi dispatch and pedestrian conversations work out in the new districts. The purple route responds to road closures, and browsing a race list can no longer change a race you've paid for.

## What was fixed

### Career, money and saves

1. **Mara's opening mission could disappear.** Finishing Dax's legal recovery first counted as your first courier paycheck, skipped Mara's bonus and unlocked listings too early. You can still do recovery first and earn $160; Mara's actual opening run still pays $140 and unlocks the next step.
2. **A failed save on Windows could silently lose the session.** Closing the window now checks whether the save succeeded. If it failed, the safe choice is to keep playing; discarding progress requires an explicit choice. Testing covered both choices and a successful retry.
3. **Windows had no way back after certain save upgrades.** When a persistent Crossroads feature expands an eligible career, the game now preserves the original before replacing it. A two-patch, 304-byte career became a 1,044-byte career with both patches intact and a byte-identical backup. This can't recreate a save an older release already overwrote.
4. **Three purchases bypassed an open Revenue case.** The Bell Lane deed ($480), Glass Crown home ($640) and Halo Bridge frame ($310) now respect the same hold as other major purchases. Smaller purchases still work, and settling the case restores access.
5. **Sleeping across midnight didn't reset daily rewards correctly.** The Signal market gift and Nightglass showcase payout now follow the day on the city clock. Sleeping from 23:00 to 01:00 starts a new claim day; merely playing longer during the same displayed day doesn't.
6. **Sleep cooled district attention twice.** A two-hour rest now takes 90 district attention down to 84, not 78. This is neighborhood attention, not an automatic escape from an active police chase.
7. **Full wallets could erase business proceeds.** If a franchise has $90 ready but your wallet can only fit $3, it pays $3 and leaves the other $87 at the desk. All three franchises use the correction.

### Driving, navigation and taxi work

8. **A parked driver's GPS ignored changing roadworks.** If Freight Parkway closes while you're watching the atlas, the purple line now reroutes without requiring you to move. In the tested case, the 574-unit direct route became a 1,106-unit detour, then returned when the road reopened.
9. **The regional GPS forgot the original city could be a shortcut.** Between Windmill and Glass Bluff it previously sent you around the coast. The same test trip now takes a 2,504-unit route through the city rather than the previously displayed 5,494 units.
10. **Signal deliveries hid booked race objectives.** A Motor Works event now shows its gate and route over a background delivery. The delivery comes back after the event; your own manually selected pin still takes priority.
11. **Diagonal roads had bad distances and missing turn arrows.** Two diagonal legs previously displayed 400 units and no turn. They now display 282 units and the correct bend direction; gentle curves don't produce spurious turn instructions.
12. **Browsing the race list could switch your paid course.** Once booked, the gates, pace car, payout and best-time record all stay with that course even if you preview another. A $75 Blufflight Five entry kept its gates and recorded the $185 win in the correct slot.
13. **Outer Loop taxi calls sent you back to the old city for every pickup.** Press T from the expansion and riders can now wait at signed, drivable curbs in six neighborhoods. Destinations can be inside or outside the old city. Pay uses the routed trip distance, and driving quality still affects tips. Original-city calls keep their existing sequence.

### People, neighborhoods and venues

14. **Outer Loop landmark pedestrians ignored E.** The six day/night crowd sites now have 24 distinct people you can approach. E or Shift+E opens the normal conversation. Only that person stops, and a change of shift won't dismiss them mid-sentence.
15. **People blinked out on curved road shoulders.** Some walking positions fell just off the playable edge. Their positions now stay on the same shoulder: the 52 missing samples in a 5,760-pose sweep dropped to zero.
16. **Close-range street encounters falsely reported an obstruction.** A nearby person in a clear street could reject the in-game Rob choice with “No clear line.” Open space now works at close range in all eight tested directions. Walls, actual cover and near-complete overlap still block it.
17. **A full satchel could block two stores and Mara's proof pickup.** At Ferry Market or Kiosk Lantern Exchange, an RSVP for a gift you couldn't carry kept intercepting E. The door or mission handoff now works, and the unclaimed gift stays available.
18. **The Night Cart charged you after it had gone.** Keeping its shop open past relocation or 05:00 closing used to allow a stale $175 purchase. The window now closes, explains why, and leaves your money alone. The daytime showroom is unaffected.
19. **The aquarium promenade drew the wrong people and poses.** Keeper Inez no longer inherits your haircut or clothes. Your own character turns in the direction they walk and stops marching when idle or pressed against the boundary.
20. **The penthouse directions named the wrong storefront.** The residence pin now sends you to **Tern Sky Club**, the name on the street and atlas, and identifies Glass Crown as the lease desk inside.

### Controls, performance and readability

21. **Quick Windows taps could go missing.** A fast Enter, E or F5 press and release between input polls now registers. A test that previously couldn't create a character completed the opening courier/taxi route with 24 short taps. Tab still opens the wheel only while held.
22. **Linux redrew the whole game for every burst of mouse movement.** In two controlled seconds, 2,000 motion events triggered 2,002 presentations before and 122 after. Hover still updates and clicks and resizes still refresh. This is less redundant rendering, **not** a claim of 94% higher FPS. Also corrected the obsolete edition name in the Linux “another instance is using this career” message.
23. **The weapon wheel hid what you had equipped.** Ferry Prism, Arc Seed and Halo Bridge each showed up as the same pistol. The wheel now names the fitted frame and displays loaded, capacity and reserve cells. The ordinary pistol, fists and separate med-patch utility are unchanged.
24. **F1 left out three useful controls.** The guide now includes Signal (P or rebound F7), the regional atlas (F8 while the map is open) and fitted-tool reload (L), without adding another permanent street HUD panel.

## What was actually tested

- **Opening and jobs:** The 3,664-frame new-career → courier → taxi route produced the expected save on both launchers. Several testers used that same route; it's counted here once. All 17 city listings were exercised at different depths, including eight branches checked through both launchers. A separate 1,302-frame keyboard drive picked up a Parkway Overlook passenger and delivered them to Wrecker Yard for $147 once. The 288-fare stress test covered every outer pickup/drop-off neighborhood and four roadwork times; those rides staged the car at the curbs rather than manually driving 288 complete journeys.
- **Driving:** 6,268 ordered road-node routes and 47,382 constituent segments were checked with the road open or closed. All five Motor Works courses were driven to the finish in a controlled game-loop fixture; ordinary traffic was disabled for those race runs. Other checks covered vehicle damage and repair, braking, chase camera, headlights, parking, recovery and ghost timing.
- **Street life and venues:** The Parkway pedestrian replay matched on both launchers over 795 input frames. Tests exercised conversation, surrender, fight, flee, reward pickup, finite police dispatch and return. The existing 40,000-roll check still gave 35.163% surrender and every payout from $15 to $155; the odds weren't changed. Interior checks covered 259 interaction routes across 15 named venues, 63 street addresses and five gated rear rooms. Vault 39, Mika and Ivo, the separate Afterimage, aquariums, the night vendor, Signal, Lantern and Bell Lane were exercised. Venue regressions included 100 full-bag store entries and 128 cart schedule transitions.
- **Money and continuity:** 4,320 franchise desk/timing combinations, the $500/$501 spend boundary, protected ATM funds, five-minute seizures, unpaid community service, save migration, failed writes and separate career slots. Lantern production already finished with its own counter open; it wasn't changed. The equipment wheel and Signal intentionally pause simulation.
- **Presentation:** Raster checks covered 79 interior views, 158 atlas states and 768 character poses. The Windows presentation test detected no black fill overlapping the game viewport in 75 blits. A separate 62-second Linux timing run still saw occasional hitches: median frame interval 16.66 ms, 95th percentile 19.04 ms, worst 113.52 ms on the shared test machine.

**Limits:** These were scripted gameplay, rendering checks and targeted stress scenarios using disposable careers. Tests ran the native Linux SDL executable and the actual Windows executable under **simulated Win32 on Linux**. There was no physical Windows 11 PC, hardware GPU/audio/controller test, uninterrupted human career, or proof covering every possible input sequence. The original game and real player saves were not used as test scratch space.

## What remains / what I'd add next

These are **not included fixes**. Ordered by their likely impact on an old-GTA-style city:

1. **Make Outer Loop traffic behave like city cars.** This is a verified gap: an expansion car blocked a driven vehicle, but a pedestrian could stand inside it and couldn't board it with E. Give those cars consistent collision, yielding and boarding without replacing the player's owned car or active job vehicle.
2. **Show where police last saw you.** A brief dispatch call or fading atlas search area based on an officer's actual last sighting would explain a pursuit without revealing every unit.
3. **Connect nightlife to the streets.** Give The Afterimage a few recognizable regulars and an optional promoter job driving someone to the next show. This is separate from the repaired outdoor landmark pedestrians.
4. **Quote a taxi fare before acceptance.** A concise radio offer with pickup neighborhood, rough distance and base fare would make choosing a trip meaningful while keeping quick-accept on T.
5. **Let players end a neighborhood taxi shift with a receipt.** Show fares, tips and collisions together instead of forcing another trip; individual rides should still pay immediately.
6. **Let the player track the Night Cart.** Add an optional last-seen time or updating objective to its existing snapshot pin. Fixing sales after closing didn't add live tracking.
7. **Put a Revenue receipt in Signal.** After a major purchase or seizure, show what changed in each wallet, the debt, next sweep and an optional pin to the Annex.
8. **Make district crossings easier to read.** Briefly show the neighborhood name on entry, reusing existing location banners and suppressing repeat popups at boundaries.
9. **Add optional frame-time diagnostics.** An exportable, short frame-time trace on a player's own PC would help distinguish game hitches from display or driver issues. Occasional slow frames remain an open performance question; don't assume mouse coalescing solved them all.

## Combined build check

All six sets of source changes were merged and retested together. The final
native Linux and Windows binaries passed the original career opening route
with the unchanged save checksum `a6727845`. Both launchers matched the new
1,302-frame taxi trip and 795-frame Parkway conversation, and passed the
full-bag shop handoff and closed Night Cart replays. On the combined build,
the Windows quick-tap, failed-close save and first expanded-save backup
tests passed; the stationary Linux atlas rerouted at 06:00; and the Linux
mouse test rendered 122 presentations per 2,000 motion packets.

The integrated gameplay stress run completed **1,200,000 fixed-step ticks
and 14,496 render passes over 48 seeds** under address, undefined-behavior
and float-cast sanitizers. The stress test itself needed one correction
to recognize the already-existing Crossroads menu scenes. This is simulated
game time, not a 40-hour real-time playthrough.

This build was packaged as **Crossroads QA** for direct download. It has
not been pushed to GitHub. The window still says **Port Vanta Crossroads**.
The executable SHA-256 checksums are:

- Windows: `70d7caddaff3d5de4750808b10915168a13a695793be5fb3dbad3de70223de31`
- Linux: `1af4d9146ecd3cb912f76a9293ac62460f65941c3cf5cebe87310baeaea95a7f`
