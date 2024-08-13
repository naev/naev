---
title: "Fabricators"
---
## Fabricators

### Independent
* **Melendez Corp.**: Focuses on large cargo-oriented trade ships and heavy engines
* **Krain Industries**: Niche fabricator that makes the Kestrel and Starbridge (and special engines)
* **Nexus Shipyards**: Makes a decent amount of ships for the Empire and their civilian counterparts. Also makes engines and stealth armour.
* **Aerosys**: Makes the Hyena
* **MilSpec**: Makes high-end cores and outfits
* **Unicorp**: lots of low-end generic armour
* **Tricon**: makes engines
* **Schafer & Kane Industries**: Heavy armours and cargo hulls
* **Red Star Organization**: Defensive armours

### Faction-specific
* **House Goddard**: Makes the Goddard and involved in the design of House Dvaered ships
* **House Dvaered**: Factional ships
* **Skull and Bones**: Makes pirate versions of many ships
* **Sirius Systems**: Factional ships
* **Soromid**: Faction ships
* **Za'lek Fabricators**: Factional ships
* **Za'lek Robotics**: Za'lek drones
<% if faction.get("Thurion"):known() then %>
* **Thurion Shipyards**: Factional ships
<% end %>
<% if faction.get("Proteron"):known() then %>
* **Proteron**: Factional ships
<% end %>

<% if player.misnDone("Operation Cold Metal") then %>
### Miscellaneous
* **Robosys**: For the collective
<% end %>
