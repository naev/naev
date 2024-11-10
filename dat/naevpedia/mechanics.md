---
title: "Mechanics Overview"
priority: 1
---
# Mechanics

Below is an overview of the mechanics covered in the Holo-Archives.

## Basics

* [Movement](mechanics/movement)
* [Landing](mechanics/landing)
* [Hyperspace and Jumping](mechanics/hyperspace)
* [Autonav](mechanics/autonav)
* [Star Map](mechanics/map)
* [Equipment](mechanics/equipment)
* [Time](mechanics/time)
* [Mass](mechanics/mass)
* [Trading](mechanics/trading)
* [Missions](mechanics/missions)
* [Energy](mechanics/energy)
* [Credits](mechanics/credits)
* [Cargo](mechanics/cargo)
* [Hailing](mechanics/hailing)
* [Weapon Sets](mechanics/weaponsets)
* [Launchers](mechanics/launchers)

## Advanced

* [Boarding](mechanics/boarding)
* [Damage](mechanics/damage)
* [Reputation](mechanics/reputation)
* [Ship Stats](mechanics/shipstats)
* [Mining](mechanics/mining)
* [Electronic Warfare](mechanics/ewarfare)
* [Effects](mechanics/effects)
* [Heat](mechanics/heat)
<% if naev.player.fleetCapacity() > 0 then %>
* [Fleets](mechanics/playerfleet)
<% end %>
<% if require("common.soromid").playerHasBioship() then %>
* [Bioships](mechanics/bioships)
<% end %>
<% if require("common.sirius").playerIsPsychic() then %>
* [Psychic Powers and Flow](mechanics/flow)
<% end %>
