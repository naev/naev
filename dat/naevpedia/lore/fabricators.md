---
title: "Fabricators"
---
# Fabricators

There are a diversity of fabricators that produce different ships and outfits.

## Independent

* **Melendez Corp.**: Focuses on large cargo-oriented trade ships and heavy engines.
* **Krain Industries**: Fabricator that makes the [Kestrel](ships/kestrel) and [Starbridge](ships/starbridge) and their assorted custom engines.
* **Nexus Shipyards**: Makes a decent amount of ships for [the Empire](lore/factions/empire) and their civilian counterparts. Also makes engines and stealth armour.
* **Aerosys**: Maker of the popular [Hyena](ships/hyena) interceptor.
* **MilSpec**: Provides high-end cores and outfits. A favourite of bounty hunters.
* **Unicorp**: Designs many accessible core outfits, and also launchers for the Caeser torpedoes, among others.
* **Tricon**: Design high-end engines optimized for speed.
* **Schafer & Kane Industries**: Manufacturer of heavy armour and cargo hulls.
* **Imperial Red Star**: An organization dedicated to relief efforts that has also designed their own line of defensive armours.

## Houses

* **House Goddard**: Makes the [Goddard](ships/goddard) and involved in the design of [House Dvaered](lore/factions/dvaered) ships.
* **House Dvaered**: Works in tandem with House Goddard to develop the ships used by [House Dvaered](lore/factions/dvaered).
* **Sirius Systems**: Besides developing the ships for [House Sirius](lore/factions/sirius), also develops the [Schroedinger](ships/schroedinger) scout ship.
* **Za'lek Fabricators**: Focuses on developing and making the ships flown by [House Za'lek](lore/factions/zalek).
* **Za'lek Robotics**: Produces [House Za'lek](lore/factions/zalek) drones.

## Other
* **Soromid**: Uses advanced genetic techniques to grow and train biological ships.
* **Skull and Bones**: Known for stealing and developing variants of any ship they can get their hands on. Almost all the materials used are from dubious sources.
<% if faction.get("Thurion"):known() then %>
* **Thurion Shipyards**: DesignFactional ships
<% end %>
<% if faction.get("Proteron"):known() then %>
* **Proteron**: Although they use an antiquated fabrication methods, they have refined them to a point they are extremely competitive with other ships.
<% end %>
<% if player.misnDone("Operation Cold Metal") then %>
* **Robosys**: Provides support and modifications for the Collective ships.
<% end %>
