# Dead Channel: Crossroads Street Life

Street Life adds the nine follow-ups from the Crossroads QA report. It keeps
the earlier fixes and uses the same career formats. Extract to a new folder;
on Windows, run **DeadChannel.exe**. The Street Life window title distinguishes
this build from Crossroads QA. Both releases use the same save directory.

| Addition | What to try |
| --- | --- |
| Outer Loop traffic | Approach an eligible car on foot and press **E**. You can get in, drive, park, and get back in. Traffic has solid bodies, yields when blocked, and follows the Parkway closure. Loans, owned cars, and mission vehicles keep their original slots. |
| Police last-seen | During a pursuit, open the atlas with **M**. After actual officer contact is lost, a dashed area briefly shows where the player was last seen, then fades. It doesn't show exact live police positions. |
| Afterimage show rides | **E** enters The Afterimage from the pink marquee west of Apartment 4. Meet Jules, Remi and Bryn on the main floor. Accept Bryn's optional job, stop at both marked curbs in an owned or licensed car, and let Jules get in and out. The clean fee is separate from taxi income. |
| Taxi radio quote | Complete an initial Dispatch fare, then stop in an owned or licensed car and press **Q** for the pickup, destination, road distances and base fare. **E** or **Enter** accepts the displayed offer; **Esc** declines. **T** still accepts instantly. |
| Taxi shift receipt | **R** on foot or while stopped shows trips, fares, tips, bonuses, impacts and actual clean cash credited. **Enter** ends the shift between fares; **Esc** closes the receipt. Trips still pay individually at drop-off. No extra payment for ending a shift. |
| Live Night Cart | Open Signal with **P** (or rebound **F7**), then choose **Night Wire → Night Cart**. The map follows the next scheduled stop, shows the last check-in when closed, and resumes at 19:00. Select the card again or clear the map pin to stop. |
| Revenue receipts | Open Signal, then press **M** or select **Revenue receipts**. See the latest major purchase and actual seizure, wallet changes, outstanding case and sweep time. Select the Annex row and press **Enter** if you want its GPS pin. Looking at the receipt alone doesn't change your route. |
| District arrival | Travel into a different district outside. Its name appears briefly once the crossing is stable. The banner won't spam when you ride the boundary. |
| Frame diagnostics | **F9** starts or stops an optional five-second trace. Once stopped, **F10** exports `frame-times.csv` in the local DeadChannel data folder. Off by default; nothing is sent anywhere. See **FRAME-TIMES.txt**. |

Promoter bookings and the taxi shift receipt are session-only. An earned
promoter fee waiting for wallet space and the latest Revenue receipts can
persist. Existing compact and expanded saves still load. If a permanent
feature expands an eligible old career, the existing compatibility backup
applies; it cannot restore a version already overwritten before the upgrade.

## Verification

The integrated build replayed the 3,664-frame opening courier/taxi route
with its original save checksum, `a6727845`. New keyboard replays on both
launchers covered a 970-frame Outer Loop traffic stop, board, drive and
reboard; the 2,274-frame Afterimage to Copper Playhouse ride ($115 once);
and a 1,320-frame quote-to-receipt taxi shift ($147 once). Both clients
also rendered the moving Cart pin, district arrival and Revenue receipt
panels. The latter replay wrote matching new saves and kept the original
compact career. The frame diagnostic passed export and retry checks.

Additional game-logic fixtures covered officer sight and ring expiry,
traffic yield and closure cases, fare declines, exact fee deferrals and
route priority. A combined sanitizer run completed 1,200,000 simulated
gameplay ticks and 14,496 render passes across 48 seeds. The native Linux
game ran through SDL. The actual Windows executable ran under simulated
Win32 on Linux, not on physical Windows hardware. No real career was used.

Executable SHA-256:
- Windows: `66e29e3dc7f1e2023d0c4993a6d14748407b349d9332184afe559360814f2bf2`
- Linux: `b51a90d2613821e4079b3efa6ed1410e8970832c85d5b5983a6f6638b01cfec3`

### Dispatch and listing polish

City Listings now turns an entire issue with **Left/Right**, keeping your row
where possible; the five-entry last issue is reachable from either side.

First-time Mercury drivers still start with a city pickup and drop. After
licensing, downtown calls can also take a local passenger out to one of the
six Outer Loop stops. Q quotes the actual selected route; the normal safe-
driving tip is still paid separately. A pedestrian hit with a rider on board
now counts on the shift receipt and forfeits that ride's tip, not its base fare.
At the clean-wallet limit, the final notice shows the amount actually credited.
Declining a call keeps the most recent closed receipt; accepting starts a new
receipt immediately, including while the first fare of the shift is in progress.
The regional GPS no longer sends you back down the exact same stretch at
a grid-to-highway join. The quoted distance uses the corrected purple line.
The minimap keeps the map key beside the north badge, leaving the whole
footer for the district or route. Long route names shorten only as needed.
Holding up your own waiting passenger cancels that booking and its clean fare.
Saying hello, failing to draw, or talking to anyone else leaves the booking alone.


### Venue receipt and service fixes

Bryn's show fees and the three Echo Bloom grades on your private loft shelves are now independent. You can deposit or retrieve those grades while waiting to collect a fee. If an older save has a record that could mean either stock or an unpaid $145/$165 ride, Bryn explains the two possibilities. Until you decide, the affected Echo Bloom grades stay protected; **M** at those shelves pins Bryn. Your other inventory is unaffected. **Back** leaves everything untouched; keep the shelf lots or redeem the matching labels as a fee, according to what you had. An identifiable old $115 fee remains redeemable. Already-overwritten stock counts can only be recovered from an intact earlier copy of your career.

At Vault 39, orders and pouring pause while Mika is away from the counter during a confrontation. A soda she has already left on the bar can still be picked up. Normal conversation and other patrons do not stop the room.
