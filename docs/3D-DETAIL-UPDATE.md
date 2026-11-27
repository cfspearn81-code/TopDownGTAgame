# Dead Channel | 3D Detail Update

## What changed

The previous C-view room renderer enlarged flat character sprites. It
no longer does that for the player or indoor cast. Rooms now render
articulated 3D people with the existing wardrobe, facing, gait and
action state. The player's edges get extra sampling at close range;
distant crowds use cheaper geometry detail, not a flat sprite fallback.
The original sprites are still used in the original overhead view.

- Entering a room eases into closer framing without moving the player or
  changing a doorway. Large solid fixtures affect framing; glass and
  railings do not shove the camera away. The helm and car keep the same
  controls. C still toggles the view and F2 offers a temporary overhead
  fallback.
- Furnishings have dimensional cushions, cabinets, racks, displays,
  counters, barware and auditorium/reef details fitted to the existing
  room layouts. Interaction markers and existing UI panels still work.
- Cars show steering, suspension and wheel motion. Boats lean and leave
  connected wakes tied to the actual movement and mooring sites.
- Streets have more detailed lights, marquee signs, curb materials
  and storefront trim. Greyreed has layered canopies, mossy rocks,
  tree roots and fallen timber, with a softer shoreline transition.

The update is a renderer and presentation change, not a new save layout,
new economy, or replacement set of player controls. Close the old game,
extract the new archive to a separate folder, and run `./DeadChannel`
on Linux or `DeadChannel.exe` on Windows. Existing careers still load.
Linux requires glibc 2.34 or newer and SDL2.

## Verification

The native SDL entrance replay toggled C outside Vault 39, entered,
walked in both directions, exited and saved. Five overhead captures
and same-mode career files matched the published build byte-for-byte.
Paired 430-frame room tours matched each other with and without C.
The room sanitizer sweep covered 238 paths, 17,036 walking ticks, and
94 renders in 78 public interiors and gated back rooms. In a paired
combat fixture, fire timing, health and the one-time cash pickup matched.
The 1,320-frame taxi route in 3D paid $147 once on both launchers;
the original 3,664-frame opening route kept checksum `a6727845`.

Other checks exercised the renderer down to the woods and shoreline,
ranger/shop navigation, boat visibility in open water, control parity,
and cosmetic wheel/helm updates. Tests used disposable careers. The
Linux host was native SDL; Win64 used the actual executable under
simulated Win32 on Linux. No claim about physical Windows hardware or
every rare state. The terrain art cache adds about 20.6 MiB of memory;
it is not a gameplay map or a new saved field.

Executable SHA-256:
- Linux: `94dcabcc9eea2af2bf6ec8e8c1d388f50ae8bcc162afc50357e4df3508247912`
- Windows: `de088abeb936f43a3cc748c40a560c9885ed720ccd1f6f80ca467a5382f14287`
