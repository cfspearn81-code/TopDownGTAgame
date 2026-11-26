# Working on Dead Channel

Read [HANDOFF.md](HANDOFF.md) before modifying the game. It describes the current
shipped build, implementation boundaries and known limitations. This repo is
for Dead Channel, the original native Windows game, not John's Room or GTA.

## Required before uploading any new build

1. Update **HANDOFF.md** first, including exactly what changed, what stayed
   compatible, known gaps, and the build/version being prepared.
2. Build and test the source; update the handoff's verification section with
   checks actually run and their results. Never report simulated Win32 or Linux
   tests as a real Windows run. Note any migration or regressions explicitly.
3. Update player-facing controls and change notes in `docs/` when relevant.
   Commit the handoff update with (or before) the matching source and build.
   Package the executable, verify the ZIP, and update the download and SHA-256
   record in `downloads/`. Never push or upload a new build with a stale handoff.
4. Do not commit real player saves, private notes, credentials, session files,
   or unrelated workspace artifacts. Do not rewrite another contributor's
   history. Reuse the published Windows-native entry point and preserve existing
   save slots and old loaders unless the owner asks for a breaking change.

Keep source, test, and packaged build status separate. Don't claim fixes are
shipped until the published artifact matches the tested binary.
