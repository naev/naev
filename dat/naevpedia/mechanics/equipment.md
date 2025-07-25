---
title: "Equipment"
---
# Equipment

Outfits play a significant role in a ship's success at whatever activity it may attempt to do.
Each ship has a fixed number of slots with different properties

## Slots

There are fundamentally four different types of slots a ship can have:

1. **Weapons**: Used for causing damage and attacking ships.
1. **Utilities**: Provides special or conditional abilities.
1. **Structurals**: Used for changing core stats of the ship.
1. **Cores**: Can be either for hulls, engines, or systems and are required to be equipped for the ship to be space-worthy.

Furthermore, slots can have special properties such as:

1. **Fighter Bays**: Modifies weapon slots, allowing the installation of fighter bays, and in some cases, do not allow equipping normal weapons.
1. **Accessories**: A special slot that only allows equipping accessory-type outfits.

## Equipment Window

The equipment window is available when [landing](mechanics/landing) on spaceports with refueling capabilities.
It allows you to not only change ships but also to change the outfits the ship has equipped.
You can also see updated statistics and properties of the ships you own.
<% if player.fleetCapacity() > 0 then %>
Additionally, you can control what ships are currently deployed to define your fleet.
<% end %>

### Auto-Equipping

The equipment window also has autoequipping functionality.
Although it does not change the core outfits, by clicking the "Autoequip" button, it will remove non-core outfits and try to find a suitable configuration for the ship.
There is some randomness in the process, so every time you click the button, you will get a slightly different configuration.
This can be used to get a ship ready to fly quickly.

## Intrinsic Outfits

Intrinisc outfits are modifications to the ship itself that do not take an outfit slot.
They are usually much more difficult to obtain, as they require extensive modifications to the ship itself.
Once installed, in general, they can not be removed nor moved to another ship.
You can view the intrinsic outfits a ship has from the equipment window when [landed](mechanics/landing) at a spaceport with refueling capabilities.
