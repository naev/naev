---
title: "Weapon Sets"
---
# Weapon Sets

Outfits can be used by assigning them to weapon sets from the information window, which you can open with the <%= "#b"..naev.keyGet("info").."#0" %> key..
The outfits are not assigned directly, but the outfit slots can be assigned to the weapon sets.
If the outfit in the slot is changed, this will not affect the weapon set bindings, which will now affect the newly equipped outfit.

There are three types of weapons sets:

1. **Switch**: Activates what weapons are fired when the primary fire button (<%= "#b"..naev.keyGet("primary").."#0" %> key) or secondary fire button (<%= "#b"..naev.keyGet("secondary").."#0" %> key) are pressed. You have to have at least a single switch weapon set.
1. **Hold**: Activates the bound outfits when the button only when held, and releasing will stop the outfits.
1. **Toggle**: Activates the bound outfits when pressed. Pressing again will turn the bound outfits off.

You have a total of 10 weapon sets, and any number of weapon slots can be bound to each weapon set.
To change the type of a weapon set, select it and then press **Cycle Mode**.
You can also use **Clear** to reset the selected weapon set, and **Clear All** to reset all the weapon sets.

## Activating

The weapon set key bindings (such as <%= "#b"..naev.keyGet("weapset1").."#0" %> key for the first weapon set or the <%= "#b"..naev.keyGet("weapset2").."#0" %>) will activate the weapon set.
In the case of the **Switch** weapon set, it will change your current active primary and secondary weapons.
For the **Hold** and **Toggle** weapon sets, it will activate or deactivate outfits accordingly.
If an outfit is activated by at least one weapon set, it will be considered active and try to turn on.

## Options

1. **Enable manual aiming mode**: Disables swivel and turret tracking mechanics. Weapons will fire straight ahead, or at the mouse location if mouse aiming is enabled.
1. **Enable volley mode**: Weapons will now fire synchronized instead of staggering. Can be useful for unleashing timed volleys.
1. **Only shoot weapnos that are in range**: Will only fire weapons when the target is deamed to be in range. Will try to compensate for the target's velocity.
1. **Dogfight visual aiming helper**: Enables a visual overlay to help with aiming. This is a global option that doesn't depend on the weapon set.
1. **Automatically handle weapon sets**: This will reset your weapon sets and try to automatically figure out the best configuration for them.

## Hints

Below are some hints to use weapon sets more efficiently.

### Afterburners and Movement Outfits

Afterburners and some movement outfits can be toggled by double tapping [movement](mechanics/movement) keys and do not necessarily have to be bound to weapon sets.

### Point Defense Weapons

Point defense weapons are off by default.
It can be useful to bind them all to a **Toggle** weapon set that you can activate when you need point defenes support.

### Fighter Bays

Similarly to Point Defense Weapons, Fighter Bays can be set to a **Toggle** weapon set and activated on take off.
When toggled on, fighters will be deployed as fast as possible.
This can be useful to heal and restock them by recalling them and having them immediately deploy again.
Additionally, when destroyed fighters are reloaded, they will also be launched again.
