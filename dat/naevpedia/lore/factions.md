---
title: "Factions"
---
# Factions

There are a diversity of factions that vie for the control of the universe.

## The Empire

* [The Empire](lore/factions/empire)
* [Great House Dvaered](lore/factions/dvaered)
* [Great House Sirius](lore/factions/sirius)
* [Great House Za'lek](lore/factions/zalek)
* [House Goddard](lore/factions/goddard)

## Independent Factions

* [Soromid](lore/factions/soromid)
* [The Frontier](lore/factions/frontier)
* [Frontier Liberation Front](lore/factions/flf)
* [Space Traders Society](lore/factions/spacetraders)
   * Mining Vrata
   * Astra Vigilis
   * Imperial Red Star
<% if faction.get("Thurion"):known() then %>
* [Thurion](lore/factions/thurion)
<% end %>
<% if faction.get("Proteron"):known() then %>
* [Sovereign Proteron Autarchy](lore/factions/proteron)
<% end %>

## Pirates

* [Galactic Space Pirates](lore/factions/pirates)
* Marauders
<% if faction.get("Dreamer Clan"):known() then %>
* Dreamer Clan
<% end %>
<% if faction.get("Raven Clan"):known() then %>
* Raven Clan
<% end %>
<% if faction.get("Wild Ones"):known() then %>
* Wild Ones
<% end %>
<% if faction.get("Black Lotus"):known() then %>
* Black Lotus
<% end %>
