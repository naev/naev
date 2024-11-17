---
title: "Electronic Warfare"
---
# Electronic Warfare

A large part of combat is not only direct damage, but electronic warfare between the evasive and sensing abilities of ships.
Electronic warfare plays a large role in determining when ships are seen, how well weapons track or lock onto ships, and stealth abilities of hips.
There are fundamentally three different levels, from coarse to fine:
1. Detection and hiding
1. Signature and tracking
1. Stealth

All electronic warfare values are based on distances, since they represent the distance at which the ship is fully detected.

## Scanning

Whenever you target a ship, your ship will begin performing a scan on it.
This is indicated by the rotating icon on the ship tab that appears in your ship's interface.
When the icon stops rotating, it indicates that the ship has been scanned.
By pressing the <%= "#b"..naev.keyGet("scan").."#0" %> key, you can call up the information about the scanned ship.
This will show you things such as the outfits the ship has and the cargo it is carrying.

Similarly, other ships will scan your ship when targeting it.
This is usually not a problem, however, if you are carrying outfits or commodities that are illegal to the faction of the ship scanning you, it may bring repercussions.
Ships scanning your are shown on the overlay and if you have illegal items, it will temporarily stop [autonav](mechanics/autonav).

## Detection and Hiding

Without modifiers, ships are detected based on [their mass](mechanics/mass).
In particular, they are detected at a distance of `mass^(1.0/1.8) * 350` which amounts to about 6,513 km for a Llama with 193 tonnes of mass, or 43,278 km for a Goddard with 5,834 tonnes of mass.
This value gets reduced by the **Detection** bonus of the detecting ship, and multiplied by the **Detection Range** of the ship being detected.
Since this value determines the base signature and stealth values of the ship, it can play a significant role in electronic warfare.

### Detecting Space Objects and Jumps

For detecting general objects such as jump points or asteroids, the detection distance has a base value of 7,500 km.
This distance can be modified by values such as how well hidden the jump is.
For space objects such as planets or stations the base value is 20,000 km, subject to the individual properties of the object.

## Signature and Tracking

Electronic warfare plays a critical war in combat too through evasion and tracking.
A ship's signature range represents how well a ship can avoid being tracked by enemy weapons and is 75% of the detection value.
The value can be further modified by the **Signature Range** modifier of the ship, and decreased by the **Tracking** bonus of the enemy ship.
Ships will be able to identify other ships if they are closer than their signature range, otherwise they will be shown an "Unknown".

Almost all forward weapons have either an amount of swivel, which allows them to aim slightly at the target.
Furthermore, turret weapons can aim the entire 360° and shoot at ships in any directions.
Weapons try to compensate for target velocities for accurate shots, however, this is reduced based on the signature range of the target ship.
In particular, weapons have a minimum tracking range, and maximum tracking range.
Any ship with a signature range above the maximum tracking range will be aimed at nearly perfectly, barring sudden accelerations.
On the other hand, any ship with a signature range below the minimum tracking range will not be locked onto, and the shoots will aim at the current position, not correcting velocity.
For in-between values it will be interpolated linearly.

Similarly, the lock-on time of launchers is modified by the signature range of the target and tracking values of the launcher.
If the target has a signature range above the maximum tracking range, the lock-on time will be unmodified.
If the target has a signature range below the maximum tracking range, it will not be able to lock on.
Similar to bolt weapons, in-between values will be interpolated linearly.

## Stealth

The stealth range of a ship is based on 25% of the detection range.
If there are no ships within the stealth range, the ship will be able to enter stealth mode.
In stealth mode, the ship will be undetectable, however, it will have its [movement](mechanics/movement) significantly reduced.
In particular, acceleration and turn speed will be reduced by 80%, while maximum speed will be reduced by 50%.

### Jumping with Stealth

When in stealth mode, it is possible to start hyperspace jumps at three times the maximum normal distance from jump points.
Additionally, when entering a system in stealth mode, the ship will drop out of hyperspace sooner instead of near the jump point.
Being within 2,500 units of jump points also lowers the stealth range by 50%, making it significantly easier to stealth.

## Scanning

Like the detection range, the scanning time of a ship is determined by its mass.
The time it takes to scan a ship is determined by `mass^(1/3) * 1.25`, which amounts to 7.22 seconds for a Llama with 193 tonnes of mass or 22.5 seconds for a Goddard with 5,834 tonnes of mass.
This value is divided by the **Detection Range** and **Signature Range** values of the ship, and multiplied **Scanned Speed** of the ship.

## System Modifiers

System modifiers such as asteroid fields and interference can further affect electronic warfare.
In particular, being in an asteroid will affect the detection range, and consequently, signature range and stealth range depending on the density of asteroids.
Furthermore, the system interference wil lower signature and stealth ranges, but not the detection range, making it harder to track and identify ships.
