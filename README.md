# Dead Channel

An original top-down, single-player Windows 11 x64 arcade crime sandbox set in
Port Vanta. This is not GTA, a GTA port, or a browser game. The city, artwork,
and game code are original to this project; the bundled font license is in
[`docs/FONT-LICENSE.txt`](docs/FONT-LICENSE.txt).

**Play:** [Download the Street Handling Windows x64 ZIP](downloads/Dead-Channel-Street-Handling-Windows-11-x64.zip?raw=true).
Extract the ZIP and run `DeadChannel.exe`. No installer, network, or account
is required. The window title reads `Dead Channel | Street Handling`.
Previous local saves still load. The driving camera looks into turns, the vehicle dash shows speed and direction, and the radar marks responding patrols.

Begin in Apartment 4, take Mara's courier job, and explore 78 enterable
buildings. Earn clean cash from legal jobs, take street contracts, drive and
repair cars, manage businesses, and navigate with the city map. The Career Files
menu shows playtime, character portraits and both cash balances. New careers
start with a character creator and 360 editable starter looks.

- [Player instructions and controls](docs/START-HERE.txt)
- [Latest changes](docs/WHAT-CHANGED.txt) and [verification](docs/QA-REPORT.txt)
- [Technical and gameplay handoff](HANDOFF.md)
- [Required instructions before publishing a new build](AGENTS.md)

## Build and test

On x86-64 Linux, install GCC, Python 3 and GNU binutils with `i386pep` support,
then run `bash scripts/build.sh` and `bash scripts/test.sh`. The output is
`build/DeadChannel.exe`. The test harness simulates Windows APIs on Linux;
a physical Windows 11 playthrough is still outstanding.
