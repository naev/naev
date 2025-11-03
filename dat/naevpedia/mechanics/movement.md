---
title: "Movement in Space"
---
# Movement

Ship movement in space is Newtonian-based where ships can accelerate with the <%= "#b"..naev.keyGet("accel").."#0" %> key and turn with the <%= "#b"..naev.keyGet("left").."#0" %> and <%= "#b"..naev.keyGet("right").."#0" %> keys.
The ships will drift without friction unless they surpass their maximum speed, which will slow down the ship until it returns to the maximum speed.
A ship's maximum speed is determined by their base speed with an added <%= fmt.number(1./constant_raw( "PHYSICS_SPEED_DAMP" )*100.) %>% of the ships acceleration.

## Afterburners and Other Movement Outfits

Movement outfits can be useful for getting away or chasing down other ships.
While outfits may be bound to [weapon sets](mechanics/weaponsets) to use them, most movement outfits can also be triggered by double tapping acceleration with the <%= "#b"..naev.keyGet("accel").."#0" %> key.
Furthermore, if supported, double tapping turn with the <%= "#b"..naev.keyGet("left").."#0" %> and <%= "#b"..naev.keyGet("right").."#0" %> keys.
