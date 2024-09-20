---
title: "Boarding Ships"
---
# Boarding

You can board [disabled](mechanics/damage) ships by getting close to them and matcing speed with them.
This can be automated by using your ship's builtin autonav functionality (<%= "#b"..naev.keyGet("approach").."#0" %>).
Once you board the ship, an interface will open that will let you interact with the ship.
Possible actions include looting, stealing outfits, or trying to capture the ship.

## Looting

You can steal credits, fuel, cargo, and outfits from ships you board.
Looting anything from a ship will lower your reputation with the faction to which the ship belongs.

## Stealing Outfits

The outfits that you can steal from a ship are random, and the number is influenced by your **Boarding Bonus** modifier.
Higher modifier values will let you steal more outfist from a ship.
The cost of repairing a stolen outfit is the base cost multiplied by `(10 + their crew) / (10 + your crew)`.

## Capturing

Capturing a ship is a three stage process:

1. Board and Capture the ship.
1. Escort the ship to a spaceport with refueling capabilities.
1. Pay to repair the ship.

The cost of repairing a captured ship is the ship's cost, including equipped outfits, multiplied by `(10 + their crew) / (10 + your crew) + 0.25`.
Additionally, factions do not take kindly to your trying to capture their ships, and you will suffer a large reputation hit when trying to capture ships of a faction.
