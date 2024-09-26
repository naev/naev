---
title: "Cargo"
---
# Cargo

Cargo plays an important role in spacefaring, be it for cargo deliveries, or for capturing objects of value.
Each ship has a maximum amount of cargo it can carry, and a [mass limit](mechanics/mass) that decreases mobility if surpassed.

## Obtaining Cargo

Cargo may be obtained from the #bCommodity Exchange#0 found on many spaceports.
It can also be obtained from missions, [mining](mechanics/mining), or [pillaging](mechanics/boarding).

## Cargo Inertia

The **Cargo Inertia** property determines how much real mass gets added to your ship per mass of cargo, and represents how efficient the ship is at carrying cargo.
For example, with -50% Cargo Inertia, 100 tonnes of cargo would only add 50 tonnes of mass to your ship.

## Managing Cargo

You can manage your cargo from the cargo tab of the information menu that you can open with <%= "#b"..naev.keyGet("info").."#0" %>.
Cargo can be dumped from both space and while landed using the cargo menu.

### Mission Cargo

If you dump mission cargo, it will abort the mission and may have consequences in some cases.

### Illegal Cargo

Some cargo may be illegal to different factions.
The illegality status will be displayed in the cargo menu.
If a faction scans you and finds you have illegal cargo, you will likely face consequences.

<% if player.fleetCapacity() > 0 then %>
### Fleet Cargo

If you have more than one ship in [your fleet](mechanics/playerfleet), your cargo will be distributed throughout all your ships, although it will start by filling your ship first.
If a ship carrying cargo is destroyed, you will lose that cargo.
Mission cargo can only be carried on your ship.
<% end %>
