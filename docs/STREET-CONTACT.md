# Dead Channel | Street Contact Update

This update starts from Street Motion
`77c6a742c1e83f41fa44d263e8bd23063ad57f90`.

## Changes

All 107 lamppost bases now have narrow collision at the ground, using the same anchors as
their artwork. Light pools and bulbs do not block movement. Both cameras use
the same physical bases. A moving car can take a normal impact; sitting with
the throttle held at one pole does not repeatedly charge damage. Previously
valid saved positions can step out of a new collision footprint on load.

Street conversations and combat now use live positions for more of the
people actually drawn in the world, including the Afterimage queue, Signal
clients, Harbor staff, Woodline contacts and hikers, and recurring venue
characters. Ordinary physical actors can take direct hits; mission and counter
actions keep their usual keys. Jobs and staff recover instead of permanently
vanishing. Portraits and scripted cutaways are still illustrations, not
characters present in the walkable scene.

## Controls

**E** opens interactions. At a shop, counter, or active job prompt, keep
using the advertised control; **Shift+E** selects a nearby person instead.
**C** switches camera, not the rules for interacting with the world.

## Verification

- The 107 pole anchors were checked without changing lamp placement or light
  art. A 402-frame impact replay boarded with E, toggled C, hit the pole,
  held the throttle for 120 ticks without another damage event, then
  reversed away. Both launchers produced the same save.
- Twenty-five formerly decorative outdoor identities now share the same pose
  for drawing, dialogue, and hit tests. The outdoor tests cover the queue,
  nine Signal clients, ranger, clerk, hikers, Jules, and the three live-camera
  subjects. A taxi passenger cannot board while down; the same fare resumes
  after recovery. A Signal handoff still pays just once.
- The new walkable room roster includes the Afterimage stage, floor, suites
  and office, and Shoal Walk aquarium. Tests exercised direct punches and
  pistol hits, actor recovery, and the four-stop aquarium award.
- The old opening tape walked through physical lamp positions. The original
  tape stays in the source; the new 3,682-frame route goes around the posts
  and still finishes with $318 clean, full health, two legal jobs and one
  taxi fare. Windows replay uses the real PE under simulated Win32, not
  physical Windows hardware.
- Existing save layouts and wallets are unchanged. Contact and injury state
  is temporary for the running session. Run
  `bash scripts/test-street-contact.sh` for the combined release checks.

## Build

Run `bash scripts/build-linux.sh` and `bash scripts/build.sh`, then
`python3 scripts/package-street-contact.py build/delivery`. Launch
`./DeadChannel` (Linux x64, glibc 2.34+ with SDL2) or `DeadChannel.exe`
(Windows 11 x64) from the extracted archive.
