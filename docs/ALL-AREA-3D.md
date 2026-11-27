# Dead Channel | All-Area 3D

Press **C** in a playable space to switch between overhead and 3D. It is
the same career, with the same movement and interaction rules. Your view
carries when you leave a building or switch between walking, driving
and the helm. Existing saves load. The build uses the existing preference
field and does not introduce a new save layout.

- **On foot:** the camera follows from above with the compass held steady.
  W and the arrow keys still move in the usual map directions. Use **E**
  to interact, **Shift+E** for a person at a shared interaction point, and
  the existing weapon, punch and wheel controls.
- **Driving:** the 3D camera follows the heading and eases out from
  cover instead of jumping back. Cars have distinct body shapes, glazing,
  wheel and lamp detail. Storefronts, curbs, trees, road markers, skid
  traces and character effects use the 3D scene.
- **Rooms:** walk the original layouts, use counters and doors normally,
  and keep the same shops and inventories. Furniture, characters and
  space markers are drawn into the perspective room. The aquarium
  promenade retains its existing horizontal movement.
- **Greyreed Wood and Lantern Quay:** paths, trees, ranger props, wildlife,
  shoreline and watercraft have visible world models. At a berth **E**
  boards, **Shift** slows the boat, and **E** moors when stopped. Signal
  and Harbor Office commissions use the same progress and payouts.
- **Panels:** map, Signal, career files, help, menus, inventory and
  dedicated equipment or ledger screens stay readable in 2D. Returning
  to the playable space restores the selected view. **F2 Low FX** uses
  overhead temporarily without discarding your C choice.

Extract the archive to a new folder and run `./DeadChannel` on Linux or
`DeadChannel.exe` on Windows. Close the older build before opening the
same career. Linux requires glibc 2.34+ and SDL2.

## Playtest coverage

Six GPT-6.2 Astra Ultra testers covered the camera, road vehicles and
characters, city, nature, rooms, and boat/gameplay paths. The paired
opening/courier/taxi replay tested 3,680 input frames per view and
finished with byte-identical careers. A separate 1,320-frame fare in
3D tested quoting, boarding, payment ($147 once), and the shift receipt
through both launchers. The original 3,664-frame route still saves as
`a6727845`.

Room coverage included 15 named venues, five gated rooms, and 63 public
storefronts. A sanitized run took 238 paths and 17,036 movement ticks,
rendered 94 perspective views, and checked doors, Mika's service,
an aquarium reward, a planter, and ledger/counter actions. A separate
426-frame live input route switched views in Apartment 4, the street,
the help screen, and The Afterimage; saves matched on both executable
hosts. Outdoor tested examples include 3D fights and collectible cash;
all three ranger quests and both planter shops; all three harbor
commissions, boat travel, and a safe save/reload. The open-ocean edge
test also verified that the player boat stays visible away from the quay.

Native Linux runs used SDL. The actual Windows PE ran under simulated
Win32 on Linux. These are representative game routes and controlled
edge cases, not physical Windows hardware or a guarantee for every
game-state combination or frame rate. Existing save layout is unchanged.

Executable SHA-256:
- Linux: `25efa8c13dea01d9568e5c1a3070121798d440a98efa858a95c6b8857e0c4d63`
- Windows: `84dc8572b7fec43be292cc49462816e3e9314b6400b869c361fd6d254016e874`
