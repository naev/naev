---
title: "Boarding Ships"
---
# Boarding

You can board [disabled](mechanics/damage) ships by getting close to them and matching speed with them.
This can be automated by using your ship's builtin autonav functionality (by pressing the <%= "#b"..naev.keyGet("approach").."#0" %> key).
Once you board the ship, an interface will open that will let you interact with the ship.
<% if player.fleetCapacity() > 0 then %>
Possible actions include looting, stealing outfits, or trying to capture the ship.
<% else %>
Possible actions include looting, or stealing outfits.
<% end %>

## Looting

You can steal credits, fuel, cargo, and outfits from ships you board.
Looting anything from a ship will lower your reputation with the faction to which the ship belongs.
This may also make nearby ships angry if they dislike your action.

The amount of loot available is determined by your **Boarding Bonus** modifier.
Higher values will allow you to obtain more loot from the ships you board.

## Stealing Outfits

The outfits that you can steal from a ship are random, and the number is influenced by your **Boarding Bonus** modifier.
Higher modifier values will let you steal more outfits from a ship.
The cost of repairing a stolen outfit is the base cost multiplied by `(10 + their crew) / (10 + your crew)`.

<% if player.fleetCapacity() > 0 then %>
## Capturing

Capturing a ship is a three stage process:

1. Board and Capture the ship.
1. Escort the ship to a spaceport with refuelling capabilities.
1. Pay to repair the ship.

The cost of repairing a captured ship is the ship's cost, including equipped outfits, multiplied by `(10 + their crew) / (10 + your crew) + 0.25`.
Additionally, factions do not take kindly to your trying to capture their ships, and you will suffer a large reputation hit when trying to capture ships belonging to any faction.
Once you successfully capture the ship, you will obtain it, outfits and all, and be able to either use it yourself or [deploy it as an escort](mechanics/playerfleet).
<% end %>
