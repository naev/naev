---
title: "Weapon Sets"
---
# Weapon Sets

Outfits can be used by assigning them to weapon sets from the information window, which you can open with the <%= "#b"..naev.keyGet("info").."#0" %> key..
You can assign outfits to your #rprimary#0 weapons, #osecondary#0 weapons, or one of 10 weapon sets.
By default, your weapon sets will be automatically managed for you, even as you add and remove weapons, however, you can override this behaviour and modify them as you wish.

You can change the outfits assigned to each weapon set by clicking on the weapon set and then clicking on the different outfit slots to toggle them.
Modifying the weapon sets will disable automatic configuration of weapon sets, however, automatic setting can be re-enabled via options.
The outfits are not assigned directly, but the outfit slots can be assigned to the weapon sets, so that even if you swap out the outfit in the slot, the configuration will be remembered.

## Activating

Primary weapons are fired by holding <%= "#b"..naev.keyGet("primary").."#0" %> and secondary weapons are fired by holding <%= "#b"..naev.keyGet("secondary").."#0" %>.
Furthermore, the weapon set key bindings (such as <%= "#b"..naev.keyGet("weapset1").."#0" %> key for the first weapon set or the <%= "#b"..naev.keyGet("weapset2").."#0" %>) will activate the weapon set.
For non-primary and secondary weapon sets, you can either tap them to toggle them on or off, or hold them to keep them on as they are held and turn off when released.

## Options

You can further customize your weapon sets with options.

1. **Enable manual aiming mode**: Disables swivel and turret tracking mechanics. Weapons will fire straight ahead, or at the mouse location if mouse aiming is enabled.
1. **Enable volley mode**: Weapons will now fire synchronized instead of staggering. Can be useful for unleashing timed volleys.
1. **Only shoot weapons that are in range**: Will only fire weapons when the target is deemed to be in range. Will try to compensate for the target's velocity.
1. **Dogfight visual aiming helper**: Enables a visual overlay to help with aiming. This is a global option that doesn't depend on the weapon set.
1. **Automatically handle weapon sets**: This will reset your weapon sets and try to automatically figure out the best configuration for them.

## Advanced Modes

You can enable advanced mode by clicking the #bAdvanced#0 button at the bottom.
If the outfit in the slot is changed, this will not affect the weapon set bindings, which will now affect the newly equipped outfit.

There are three types of weapons sets:

1. **Default**: .Behaviour depends on whether the key is tapped or held. When tapped, it will behave like a **Toggle** weapon set, while when held, it will behave like a **Hold** weapon set.
1. **Hold**: Activates the bound outfits when the button only when held, and releasing will stop the outfits.
1. **Toggle**: Activates the bound outfits when pressed. Pressing again will turn the bound outfits off.

You have a total of 12 weapon sets, including primary and secondary, and any number of weapon slots can be bound to each weapon set.
To change the type of a weapon set, select it and then press **Cycle Mode**.
You can also use **Clear** to reset the selected weapon set, and **Clear All** to reset all the weapon sets.
