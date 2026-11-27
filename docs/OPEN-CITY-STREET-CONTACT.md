# Dead Channel: Open City / Street Contact

This combined build preserves the complete Open City
(`07e99b9813823a765c83ddc2035412123c0ac409`) and incorporates the
latest published Street Contact (`3a6536f4f513c2d824c491752501b30993db6d5b`).
They share All-Area 3D (`223bf999494088d3a5eafcaad64a044143cbeaf6`).

## Explore and play

- **Northpoint Headland:** Use **M**, then **F8**, and take either branch
  from Glass Bluff. All 93 building addresses work in overhead and **C**
  view. There are 14 newly accessible buildings, not 14 new storylines:
  not every room is a shop or mission. [Map and doors](OPEN-CITY-MAP.md).
- **Street Contact:** Keep **E** for jobs, counters, and signed entrances;
  **Shift+E** selects a nearby person. The 111 short lamp bases are solid.
  The light pools and lens artwork are not. You'll find four new poles
  around Northpoint; their bases and street-side faces stay outside lanes
  and clear of the new doors. The downtown and dock anchors are unchanged;
  some Outer Loop shoulders shifted slightly with the wider road edges. [Street Contact guide](STREET-CONTACT.md).
- **Style and Shade:** Choose **Wardrobe** in Pause or start a New Career;
  **Tab** opens Style Lab. At Saltmarsh Cinema's signed west porch, use
  **Enter** to open Shade Supply. Its pin is in Signal Settings.
- **The Afterimage:** Both views use the richer Open City floor, office,
  stage, and velvet booths, including distinct costumes. The performers
  and venue staff also retain Street Contact's physical positions and
  responses. The club redesign does not replace the upstream 3D art of
  the other venues or change their scripts.
- **At the helm:** **E** boards, **Shift** slows, and **E** moors. There
  are hull fittings, foam, navigation lights, heading and condition.
  The eastern sea channel uses the actual hull for Atlas clearance.
  The maneuvering, collision, drifting, traffic and save-on-shore rules
  have not been retuned.

## Saves and launch

No new file type or layout. Compact and extended careers from either
predecessor are supported. The optional wardrobe and gear bits remain
separate from older housing and equipment. A location occupied by
new scenery or a repositioned shoreline tries nearby dry footing.
No career transfer is required: extract into a new folder and resume.

Linux x86-64 requires SDL2 plus glibc 2.34 or newer; run `./DeadChannel`.
Windows 11 x64: extract the ZIP and open `DeadChannel.exe`. Windows tests
use the exact x64 PE under simulated Win32, not a physical Windows PC.

## Build and verify

```sh
bash scripts/build-linux.sh
bash scripts/build.sh
DC_QA_BUILT=1 bash scripts/test-street-contact.sh
DC_QA_BUILT=1 bash scripts/test-open-city.sh
python3 scripts/package-open-contact.py downloads
python3 scripts/qa/verify_open_contact_archives.py downloads
```

The current opening is the genuine Street Contact 3,682-frame tape
(`7dfa7bce`). The 3,664-frame All-Area 3D tape is preserved as history:
its old route walked through positions that now contain solid poles.
Sources and docs are identical across the two archives.
