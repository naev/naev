---
title: "Damage"
---
# Damage

Weapons deal damage to lower the shields and armour of enemy ships.
If the target ship has shields up, damage is first dealt to shields.
When shields are down, damage will be dealt to armour.
Finally, when the target ship reaches 0 armour, it will be destroyed.

Apart from standard damage, weapons can deal disable damage, which instead of hurting armour, it applies stress, which can disable the ship.
Disabled ships [can be boarded](mechanics/boarding).

## Absorption and Penetration

Absorption reduces the amount of damage taken by a ship by a percentage amount.
However, this reduction can be nullified by penetration, which is directly subtracted from the absorption.
Note that penetration can not increase the damage past 100%, and absorption can only reduce the damage to 0%.

## Disable Damage and Stress

Disable damage is the only way to disable ships, by applying stress.
When a ship's stress is larger than the available armour, it becomes disabled, and [can be boarded](mechanics/boarding).
Unlike regular damage, disable damage can penetrate shields, however, it suffers a large penalty when shields are up.
In particular, it will deal 50% stress when the target has 100% shields, and 100% stress when the target has 0% shields.
Disable damage is also affected by absorption and penetration.

Finally, ships will also be disabled when their armour goes under <%= fmt.number(constant_raw( "PILOT_DISABLED_ARMOUR" )*100) %>%.
However, this is much less reliable than using weapons that cause disabling damage.

## Damage Types

There are different damage types that differentiate how much damage is done to shields versus armour, and the amount of knockback.
For example, kinetic damage does 80% damage to shields, 115% damage to armour, and 100% knockback.
On the other hand, ion damage does 130% damage to shields, 80% damage to armour, and has 10% knockback.
Using the best damage type can play a critical role in winning fights.

## Shields Down and Regeneration

When a ship's shields get knocked offline, it takes <%= constant( "PILOT_SHIELD_DOWN_TIME" ) %> seconds for the shields to start regenerating again, and can be reduced with lower **Shield Down Time**.
Any damage during this window will restart the time it takes to regenerate shields.
When shields get back up there they will regenerate faster during the first 3 seconds.
