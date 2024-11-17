---
title: "Ship Stats"
---
# Ship Stats

There are many different important stats for ships that define their behaviour when flying in space.
These are all visible from the [equipment](mechanics/equipment) panel.
The final ship stats are defined based on the intrinsic ship stats, equipped outfits, and other sources such as temporary effects.

## Core Stats

The core stats define basic properties of the ship.

* [Model](ships): The name of the ship model.
* [Class](ships/classes): The ship class.
* [Value](mechanics/credits): The value of the ship including all the [outfits](outfits) equipped on it.
<% if naev.player.fleetCapacity() > 0 then %>
* [Fleet Capacity](mechanics/playerfleet): Determine how much fleet capacity a ship uses when deployed in a [fleet](mechanics/playerfleet).
<% end %>
* [Crew](mechanics/boarding): The crew the ship has. This plays a role when [boarding](mechanics/boarding) other ships.

## Movement Stats

Movement is based on Newtonian physics, however, ships have a maximum speed limit when acceleration is not being applied.

* [Mass](mechanics/movement): How heavy the ship is. If the mass of a ship goes over the [mass limit](mechanics/mass) of the engine, it will receive a penalty to [movement](mechanics/movement).
* [Jump Time](mechanics/time): Determines how long much [time](mechanics/time) it takes for the ship to go through [hyperspace](mechanics/hyperspace).
* [Accel](mechanics/movement): Specifies how fast a ship can accelerate. While accelerating, a ship can go over the [max speed](mechanics/movement).
* [Speed](mechanics/movement): Determines the max speed of the ship when not accelerating.
* [Turn](mechanics/movement): Specifies how fast the ship can rotate.
* [Time Constant](mechanics/movement): Determines how fast time feels like it is moving. A value of 100% indicates that time progresses normally, a value of 120% would indicate that 1.2 seconds of game time happen in 1 second of real time.

## Electronic Warfare

Electronic warfare determines how ships see and target each other.

* [Detected at](mechanics/ewarfare): The distance at which the ship will be detected without detection bonuses.
* [Signature](mechanics/ewarfare): Determines how well weapons track the ship. Any weapon with a lower maximum tracking value will be able to accurately target the ship. Any weapon with a higher minimum tracking value will have significant difficulties when tracking the ship. Also affects the time it takes for launcher weapons to lock on to the ship.
* [Stealth at](mechanics/ewarfare): The minimum distance needed from the nearest non-ally ship to be able to enter [stealth](mechanics/ewarfare).
* [Scanning Time](mechanics/ewarfare): The amount of time it takes other ships to scan this ship and see the details of the cargo it has.

## Health Stats

* [Absorption](mechanics/damage): Determines how much incoming damage is mitigated, however, can be overcome by weapon penetration.
* [Shield](mechanics/damage): The ship's first line of defense. Shields naturally regenerate and provide the ship with a first line of defense, absorbing weapon fire that would otherwise damage the hull.
* [Armour](mechanics/damage): The last line of defense of a ship. When armour is completely depleted, the ship will begin to combust and finally, explode.
* [Energy](mechanics/energy): Primarily used to power weapons and other outfits. A ship is not able to take off with negative energy.
* [Cargo Space](mechanics/cargo): Determines how much cargo mass the ship can fit.
* [Fuel](mechanics/hyperspace): Fuel is used for jumping through hyperspace to cross astronomical distances in a short time. The amount of fuel used per jump depends on the ship size.

## Additional Stats

There are many additional ship stats you will encounter on different outfits and ships, such as modifiers for **Ammo Capacity** or **Jump Time**.
