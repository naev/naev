---
title: "Factions"
---
# Factions

## The Empire

* [The Empire](docs/factions/empire)
* [Great House Dvaered](docs/factions/dvaered)
* [Great House Sirius](docs/factions/sirius)
* [Great House Za'lek](docs/factions/zalek)
* [House Goddard](docs/factions/goddard)

## Independent Faction

* [Soromid](docs/factions/soromid)
* [The Frontier](docs/factions/frontier)
* [Frontier Liberation Front](docs/factions/flf)
* [Space Traders Society](docs/factions/spacetraders)
   * Mining Vrata
   * Astra Vigilis
   * Imperial Red Star
<% if faction.get("Thurion"):known() then %>
* [Thurion](docs/factions/thurion)
<% end %>
<% if faction.get("Proteron"):known() then %>
* [Sovereign Proteron Autarchy](docs/factions/proteron)
<% end %>

## Pirates

* [Galactic Space Pirates](docs/factions/pirates)
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
