# Woodline & Lantern Quay Stability

This edition combines GitHub's Street Life Stability Update
(`5ba89d979c7e49e91f94acd8aca4de5d67b65f51`) and the complete,
previously delivered Woodline & Lantern Quay builds. Their common source
is published Crossroads (`a545bd8fd6720bef690ee1410fb70ba97a2d15dd`).
The older archives and their exact checksums are unchanged.

## In game

- Regional taxi and dispatch, interactive outskirts pedestrians, Afterimage
  show rides, Signal receipts, Night Cart tracking, and local frame-time
  capture remain available. Cars in the outer districts use the same hull,
  controls and twin front beams as cars in the city.
- Walkable hills and harbor shores, Greyreed Wood's three ranger quests,
  collectible fictional mushrooms, both grow-shop planter benches, wildlife
  and rare ambient-only Seawall incidents are all retained.
- Lantern Quay has three working dock pairs, a free sliding launch, ordinary
  traffic and occasional patrol, and three fictional Harbor Office commissions.
  See [Lantern Quay](LANTERN-QUAY.txt) and [Harbor Commissions](HARBOR-COMMISSIONS.txt).
- Regional residents keep individually spaced walk routes, respond to
  conversation, and avoid the deep asphalt where road shoulders meet.
- Afterimage retains its detailed cast, outfits and business systems as well
  as named show-rider roles. Vault 39 is a separate venue.

## Careers and migration

No change to the overall save size or revision: older Rev2–10 files and
both 304- and 1,044-byte Rev11 variants are supported. Keep the previous
folder and an independent copy of your saves before updating. Installing
to a new folder does not create a new save directory.

The delivered Woodline build stored its ranger journal, collection and
beds in four garage words also used for seizure receipts in Street Life
Stability. The combined read path recognizes the older receipt's exact
signature and migrates it into separate unused words. It does not mistake
legal Woodline flags, even zero flags with active beds, for a receipt. A
byte-identical copy of an eligible older receipt-bearing file is kept before
its first rewrite. New receipts and forest data no longer share fields.
The later graded shelf/promoter fee protection remains in effect.
No software can restore data already overwritten in an older version.

A boat can save while afloat, but reopens at the last safe dock. Mission
state survives; hull damage and momentum do not. Do not reopen an actively
updated career in an older executable unless you restore a separate old save
copy. This is a forward compatibility guarantee, not a downgrade guarantee.

## Checks

The native Linux binary is exercised under SDL2 and the exact Windows PE
under a simulated Win32 harness on Linux, not on a physical Windows machine.
Automated test saves are separate from real player data. The combined
test matrix retains the original 3,664-frame opening courier/taxi digest,
a complete regional fare and Afterimage show ride on both launchers, the
seven physical roadside vehicles and 24 named pedestrians, and exact
dual-host receipt checkout. For the new branch this includes the Ranger
and both store benches, all three harbor commissions, dock-safe save and
reopen, twin headlight patterns and wardrobe/guide frames. The graph checks
cover Parkway schedules and the three new waterfront pins without adding
them to the original six taxi ranks.

No older save fixture was modified in place. Separate exact predecessor
clients authored forest-only, planter-only, harbor, revenue-threshold and
aged sweep samples. Reopening verifies unchanged health, funds, job fields
and the byte-exact receipt backup. Mixed saves retain a new receipt while
old plants and a harbor case remain active.

Start with `scripts/test-woodline.sh`, `scripts/test.sh --tour`, and the
tests noted in [Street Life Stability](STABILITY-UPDATE.md). Built
executables of this distribution (SHA-256):

- Linux ELF: `7657d01c36b1d64515ba9d7209a31231b2df102c7c92c6828b9c86349d5c7fde`
- Windows PE: `95ee6f031f29abe5da5ea677955488b6e10bede7a19d4bcb31dd2fe74e7dee76`

Build with `bash scripts/build-linux.sh` and `bash scripts/build.sh`.
Build both before `python3 scripts/package-woodline-stability.py`.
Verify the exact archives with `python3 scripts/qa/verify_woodline_stability_archives.py`.
