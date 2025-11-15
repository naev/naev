---
title: "Factions"
---
# Factions

There are a diversity of factions that vie for control of the universe.

## The Empire

* [The Empire](lore/factions/empire)
* [Great House Dvaered](lore/factions/dvaered)
* [Great House Sirius](lore/factions/sirius)
* [Great House Za'lek](lore/factions/zalek)
* House Goddard
* House Yetmer-O'rez

## Independent Factions

* [Soromid](lore/factions/soromid)
* The Frontier
* Frontier Liberation Front
* Space Traders Society
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
