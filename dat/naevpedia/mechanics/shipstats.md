---
title: "Ship Stats"
---
## Ship Stats

There are many different important stats for ships that define their behavour when flying in space. These are all visible from the [equipment](mechanics/equipment) panel.

* [Model](ships): The name of the ship model.
* [Class](shipstats/class): The ship class.
* [Value](credits): The value of the ship including all the [outfits](outfits) equipped on it.
<% if naev.player.fleetCapacity() > 0 then %>
* [Points](mechanics/playerfleet): Determine how much fleet capacity a ship uses when deployed in a [fleet](mechanics/playerfleet).
<% end %>
* [Crew](mechanics/boarding): The crew the ship has. This plays a role when [boarding](mechanics/boarding) other ships.

## Movement Stats

* [Mass](mechanics/mass): How heavy the ship is. If the mass of a ship goes over the [mass limit](mechanics/mass) of the engine, it will receive a penalty to [movement](mechanics/movement).
* [Jump Time](mechanics/time): Determines how long much [time](mechanics/time) it takes for the ship to go through [hyperspace](mechanics/hyperspace).
* [Accel](mechanics/movement): Specifies how fast a ship can accelerate. While accelerating, a ship can go over the [max speed](mechanics/movement).
* [Speed](mechanics/movement): Determines the max speed of the ship when not accelerating.
* [Turn](mechanics/movement): Determines how fast the ship can rotate.
* [Time Constant](mechanics/movement): Determines how fast time feels like it is moving. A value of 100% indicates that time progresses normally, a value of 120% would indicate that 1.2 seconds of game time happen in 1 second of real time.

## Electronic Warfare

* [Detected at](mechanics/ewarfare):
* [Signature](mechanics/ewarfare):
* [Stealth at](mechanics/ewarfare):
* [Scanning Time](mechanics/ewarfare):

## Core Stats

* Absorption
* Shield
* Armour
* Energy
* Cargo Space
* Fuel

## Additional Stats

* Foo
