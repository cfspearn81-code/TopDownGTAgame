# Dead Channel | 3D Street Motion Update

This update layers on top of 3D Detail, from source `08987268a56609b62293e62653dcda7b82d70283`.
It keeps the existing lamppost models. The C camera still works on foot, in
walkable rooms, in cars, and at the helm. F2 can temporarily switch to the
less demanding overhead renderer. Map, menu, and interaction bindings are
unchanged.

## Player-visible changes

- Outer Loop fixtures use checked shoulders instead of a fixed offset from an
  intersection. The same 30 posts and three dry pier lights keep their models
  and appearance. The shoulder check uses the whole road union, including later
  connectors and junctions without changing road collision or saved positions.
- C view samples a separate ground layer, without the flat artwork belonging
  to extruded objects. Real shadows, light pools, curb and road markings remain.
- C-view walking and sprinting are slower (69% of the former street walking pace, 75% for a
  sprint at steady state), with gradual acceleration, weight transfer and
  strides driven by the distance actually traveled. F2 changes
  rendering only; switching C off restores the original overhead pace.
- The road camera stretches and looks ahead as speed builds, swings gently
  around turns, and compresses briefly under braking or a bump. It looks back
  when reversing without rolling the horizon or passing through scenery.
- Civilians commit to stepping clear of a crossing or waiting on a narrow
  sidewalk, rather than stepping away and immediately taking the same spot
  again. Their headings and stride settle smoothly instead of snapping.
- Sprint gates, service bays, closures, roadside signs, and the Night Signal
  cart have perspective-specific models and cues; offscreen routes remain
  readable. Job triggers, prices and rewards were not changed.

## Verification and limitations

The six GPT-6.2 Astra Ultra review lanes developed and checked the fixes
independently. The combined gate is `bash scripts/test-3d-street-motion.sh`.
The Windows executable is exercised under Win32 simulation, not on physical
Windows hardware. The separate ground layer replaces the old outer-only
material cache and uses about 15.44 MiB more static memory. This is a visual
and motion update, not a promise of a particular frame rate.

The ground audit checked 104 overlapping park crowns, 190 root marks, 10
harbor structures, 49 lamp stamps, and 15 named-building stamps. It also
checked that 102 actual tree shadows and the loading stripes survived. The
lamppost audit tested all 30 Outer Loop anchors and three pier fixtures; 27
earlier anchors were inside a widened road envelope.

The release regression cases include a 414-tick test of acceleration,
turning, wall clearance, braking, reversing and C/F2 transitions; a
14,400-tick downtown foot-traffic test plus 24 expanded-road walkers;
and 238 indoor routes through 15 named venues, 63 public interiors and
staff areas. The change does not exempt pedestrians from the same real
parked-car and crosswalk collision checks. Over a separate 3,600-tick
comparison, repeated abrupt successive turns dropped from 7,210 to 0 and
physical back-and-forth reversals dropped from 4,988 to 0. Two reproduced
crossing stand-offs now finish. The launcher replays include
the 3,664-frame opening, the previous Freight Parkway fixture position,
C/F2 movement, Vault 39 and a real $147 taxi payout.

The older exact-input C-vs-overhead walking tapes are not a visual-only
comparison: C now intentionally asks the normal collision solver to walk
less far for the same time. The C-specific foot tests measure this difference
and reach the same doorways using adjusted travel times. Driving rules and
existing career structures are unchanged.

## Build

Run `bash scripts/build-linux.sh`, `bash scripts/build.sh` and
`python3 scripts/package-3d-street-motion.py build/delivery`. Extract the whole
archive and run `./DeadChannel` (Linux, SDL2 and glibc 2.34+) or
`DeadChannel.exe` (Windows 11 x64). Both archives include the source and
instructions. Existing careers can be opened by the update; test replays use
temporary careers only.
