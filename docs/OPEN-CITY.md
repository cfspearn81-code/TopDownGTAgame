# Dead Channel: Open City

This historical Open City guide also applies to the combined
[Open City: Street Contact](OPEN-CITY-STREET-CONTACT.md) release.
Open City builds on All-Area 3D. Your slot, money, businesses, job records,
controls, camera preference, and existing street addresses stay intact.
Open City does not change how boats accelerate, brake, turn, drift or take
damage. You can resume an old career without starting over.

## Get started

- **Explore:** Open the atlas with **M**, then **F8** for the wider map.
  Follow either branch off Glass Bluff to Northpoint Headland. Ten road
  links form a loop around Northpoint Green. The wider outer roads retain
  their existing centers; boat slips and quay lanes did not move.
  Walk up to a signed entrance and press **E**. **C** changes your view
  inside and out. See [the Northpoint map and address list](OPEN-CITY-MAP.md).
- **Appearance:** Select **Wardrobe** from Pause in an existing career or
  customize a new character before starting. **Tab** opens Style Lab for hats,
  hair, accessories, facial details and outerwear. No purchase is required.
- **Shade Supply:** Go to the signed side door west of Saltmarsh Cinema in
  Glass Mile and press **Enter**, or pin it first from Signal Settings
  (**P** or **F7**).
  Its fictional wearables only affect the small game systems described
  on their cards. They are not a new police evasion or combat system.
- **The Afterimage:** From Apartment Four, exit to the street and walk left
  to the pink marquee. Press **E**. Other businesses and interiors keep their
  existing art style and service options.
- **Take the helm:** Use **E** at a marked Lantern Quay boat landing. The
  redesigned helm shows compass heading, speed and hull condition; look for
  the changed launch, deck fittings, vessel lights and foam. **Shift** slows
  the launch, then **E** moors at a marked berth. **C** swaps views without
  moving the boat. As before, saving afloat resumes on dry land.

There are **93 enterable building addresses**: 79 existing locations and
14 newly opened doors. The new ones are ten Northpoint rooms, three quay
buildings and the Greyreed Ranger Hut. The basic rooms have their own
furnishings and interactive fixtures, not a separate quest at every address.
Cranes, containers, utility equipment and other outdoor props are not
buildings. Existing keycard and job gates still apply. The exposed east
coast connects to the old harbor water; Mirror Tarn is still inland, and
you cannot sail beyond the playable chart.

## Installing

On Linux x86-64, extract the archive and run `./DeadChannel` inside the
`Dead-Channel` folder. SDL2 and glibc 2.34 or newer are required. On Windows
11 x64, extract the ZIP and open `DeadChannel.exe`. Keep the old install in
its own directory if you want it around. Career files are already stored
separately; don't manually copy an executable over an actively running game.

The save file layouts are unchanged: 304-byte compact careers and
1,044-byte extended Rev11 still load. Style Lab and Shade only use
previously unused bits. If a new building or shoreline occupies a saved
spot, the game tries nearby solid ground. Older executables do not display the new
wardrobe, wearables, or places. Before downgrading, back up your saves rather
than opening the only copy in an older build.
