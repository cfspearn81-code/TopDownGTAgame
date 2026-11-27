# Dead Channel | Woodline & Lantern Quay Stability

The latest native build combines [Street Life Stability](docs/STABILITY-UPDATE.md)
with [Greyreed Wood](docs/WOODLINE.txt) and the [Lantern Quay harbor](docs/LANTERN-QUAY.txt).
See [integration and save compatibility](docs/WOODLINE-STABILITY.md).

- **Windows:** [Windows 11 x64 ZIP](downloads/Dead-Channel-Woodline-Lantern-Quay-Stability-Windows-11-x64.zip); extract and run `DeadChannel.exe`.
- **Linux:** [Linux x86-64 archive](downloads/Dead-Channel-Woodline-Lantern-Quay-Stability-Linux-x64.tar.gz); extract and run `./DeadChannel`. Requires glibc 2.34+ and SDL2.
- [SHA-256 checksums](downloads/SHA256SUMS.txt) · [Player guide](docs/START-HERE.txt).

Use **M**, then **F8**, for Greyreed Wood and Lantern Quay. Use Signal
(**P** or **F7**) to open Night Wire commissions. **E** boards at a dock;
**Shift** slows the launch. Existing Street Life, Crossroads and other
[releases remain in downloads](downloads/).


**Base Village:** Leave Apartment 4 and walk straight left to the pink
Afterimage marquee. Press **E** to enter directly. Vault 39 is still its
own nightclub with Mika's bar and Ivo's contracts. **P** opens Signal
(**F7** if rebound); use Night Wire to pin the buyable, upgradable Trap
House at Bell Lane. **Enter** at its side door.

**Drive the Outer Loop:** **C** switches to the optional behind-car chase
view, **M** opens the atlas and **F8** expands it to new coastal, hill and
highway roads. Cars take one-third less collision damage at the same
0–100 condition scale. Visit Nightglass Motor Works for parts and races.
The original street-lamp look is restored; rain, fog and wet roads remain.

After the first courier shift, press **F6** to open 17 city listings. Start with **Side B Before Sign-Off** at the DJ rack in Blue Hour Tram Hall. [City listings and controls](docs/CITY-LISTINGS.txt).

Press **E** by a person to say hello, stay quiet, or rob them in-game.
Use **Shift+E** when a counter, fare, or other hotspot also uses E.
Only the person you selected stops while the choices are open.
Everyone has 50 greetings without a repeat. A robbery needs an owned
pistol; the player draws without firing. A person either hands over
$15–$155 in dirty cash (35% odds) or fights back. Walk over dropped cash
after a fight, or get away without taking anything.

Vault 39 is now a working nightclub. Press **E** at Mika's bar to order a
free house soda; wait through the current customer's order and press **E**
to take it. Ivo handles contracts past the bar, behind the recessed
**Employees Only** door. People and vehicles have updated in-game models. See [Vault 39 controls](docs/START-HERE.txt).

The Windows display fix still limits black fills to letterbox bars. Existing
careers, med patches, and Base Village ownership are supported.

Hold **Tab** to open the weapon wheel. Point near fists or your owned
pistol, then release **Tab** to equip. While holding **Tab**, press **V**
to heal with one stocked med patch. The count and V reminder are on the
separate lower-right utility, outside the weapon ring. Buy patches at Ferry Market while at
full HP to stock them. Opening a save from an older build is supported.

On foot, an equipped pulse pistol sits in a compact bent-elbow stance.
The slide keeps its width when facing up, down or diagonally.
When you head up the screen, the pistol rests beside the shoulder and
stays clear of your character's head through walking, sprinting and recoil. The weapon
still moves with your stride and recoil, and the flash and street trace
start at the visible barrel tip. At a wall or car, the pistol lowers
beside your hip until you clear the obstacle. It will not fire while
lowered. Cars do not allow firing; indoor firing only occurs in an active NPC fight.

Press **E** to board or exit cars. **Enter** still confirms menus and boards trams.

Earlier download archives remain available, including the
[Conversation Hold Windows ZIP](downloads/Dead-Channel-Conversation-Hold-Hotfix-Windows-11-x64.zip)
and [Conversation Hold Linux archive](downloads/Dead-Channel-Conversation-Hold-Hotfix-Linux-x64.tar.gz).
Both current launchers play offline; the optional official YouTube Music link
opens the desktop browser, not an in-game stream.

Create a character, start in Apartment 4 and take Mara's first courier job.
Explore the original city, Base Village and the expanded Outer Loop. Earn clean cash
from legal shifts and businesses, take street contracts, drive and repair cars,
and use the atlas to pin destinations. The game tracks separate clean and dirty
wallets, a protected bank balance, business stock, and a 24-minute game day.
New careers have 360 editable starter looks; existing Rev2–10 saves and stocked Rev11 saves are supported.

- [Player instructions and controls](docs/START-HERE.txt)
- [Release notes](docs/WHAT-CHANGED.txt) and [test report](docs/QA-REPORT.txt)
- [Project summary and handoff for future maintainers and AIs](HANDOFF.md)
- [Required handoff update before publishing any new build](AGENTS.md)

## Build and test

On x86-64 Linux with GCC, Python 3 and GNU binutils supporting `i386pep`:

```sh
# Native host (SDL2 runtime or SDL2 development package)
bash scripts/build-linux.sh
bash scripts/test-linux.sh

# Windows cross-build
bash scripts/build.sh
bash scripts/test.sh --tour
```

Outputs: `build/DeadChannel` and `build/DeadChannel.exe`. Both launchers use
the same engine and support Rev2–10 plus both older Rev11 layouts. Windows saves use
`%LOCALAPPDATA%/DeadChannel`; Linux saves use
`${XDG_DATA_HOME:-$HOME/.local/share}/DeadChannel`. Close both builds and back up
the target saves before copying between platforms. No automatic sync.

Linux automated testing runs the exact ELF using SDL's software renderer and
dummy video driver. Windows automated testing runs the exact PE with simulated
Win32 on Linux. Interactive desktop/physical Windows testing remains open.
Package verified builds with `python3 scripts/package-woodline-stability.py`.
Before publishing another build, update [HANDOFF.md](HANDOFF.md) for the next
maintainer and verify the matching archives/checksums.
