# Dead Channel

An original top-down, single-player Windows 11 x64 arcade crime sandbox set in
Port Vanta. This is not GTA, a GTA port, or a browser game. The city, art and
code are original to this project; see the [bundled font license](docs/FONT-LICENSE.txt).

**Play:** [Download Easy Entry for Windows 11 x64](downloads/Dead-Channel-Easy-Entry-Windows-11-x64.zip?raw=true).
Extract the ZIP and open `DeadChannel.exe`. The title reads `Dead Channel | Easy Entry`.
No installer, network or account is required. Press **E** to board or exit cars.
**Enter** still confirms menus and boards trams. [ZIP checksums](downloads/SHA256SUMS.txt) and [older builds](downloads/).

Create a character, start in Apartment 4 and take Mara's first courier job.
Explore 78 enterable city buildings and four gated rear rooms. Earn clean cash
from legal shifts and businesses, take street contracts, drive and repair cars,
and use the atlas to pin destinations. The game tracks separate clean and dirty
wallets, a protected bank balance, business stock, and a 24-minute game day.
New careers have 360 editable starter looks; existing Rev2–10 saves are supported.

- [Player instructions and controls](docs/START-HERE.txt)
- [Release notes](docs/WHAT-CHANGED.txt) and [test report](docs/QA-REPORT.txt)
- [Project summary and handoff for future maintainers and AIs](HANDOFF.md)
- [Required handoff update before publishing any new build](AGENTS.md)

## Build and test

On x86-64 Linux with GCC, Python 3 and GNU binutils supporting `i386pep`:

```sh
bash scripts/build.sh
bash scripts/test.sh --tour
```

Output: `build/DeadChannel.exe`. The suite checks the native executable under
simulated Win32 services on Linux. Physical Windows 11 input and display
testing remains outstanding. Before publishing another build, update
[HANDOFF.md](HANDOFF.md) with its changes and verify the matching ZIP and checksum.
