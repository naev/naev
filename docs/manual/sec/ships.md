# Ships

TODO

## Ship Classes

Ships classes have an intrinsic size parameter accessible with the `ship.size()` Lua API. This is a whole integer number from 1 to 6.

In \naev, small ships (size 1 and 2) use small core slots and are meant to be fast and small. Medium ships (size 3 and 4) use medium core slots and are still agile, while being able to pack more of a punch. Large ships (size 5 and 6) are slow hulking giants with heavy slots meant to dominate. There is always a trade-off between agility and raw power, giving all ships a useful role in the game.

Ships are also split into two categories: civilian and military. Civilian ships are meant to focus more on utility and flexibility, while military ships focus more on combat abilities.

An overview of all the ship classes is shown below:

* **Civilian**
    * **Yacht**: very small ship often with only few crew members (size 1)
    * **Courier**: small transport ship (size 2)
    * **Freighter**: medium transport ship (size 3)
    * **Amoured Transport**: medium ship with some combat abilities (size 4)
    * **Bulk Freighter**: large transport ship (size 5)
* **Military**
    * **Small**
        * **Scout**: small support ship (size 1)
        * **Interceptor**: ultra small attack ship (size 1)
        * **Fighter**: small attack ship (size 2)
        * **Bomber**: missile-based small attack ship (size 2)
    * **Medium**
        * **Corvette**: agile medium ship (size 3)
        * **Destroyer**: heavy-medium ship (size 4)
    * **Large**
        * **Cruiser**: large ship (size 5)
        * **Battleship**: firepower-based extremely large ship (size 6)
        * **Carrier**: fighter bay-based extremely large ship (size 6)

Note that it is also possible to give custom class names. For example, you can have a ship be of class `Yacht`, yet show the class name as `Luxury Yacht` in-game.

## Ship XML

* `name`: Ship name, dispayed in game and referenced by `tech` lists.
* `points`: Fleet point value.
* `base_type`: Specifies the base version of the ship, useful for factional or other situational variants. (For example, a Pirate Hyena would have the "Hyena" base type.
* `GFX`: Name of the ship graphic in .webp format.
* * `size`: The ship sprite's resolution in pixels ^2. For example, `size=60` refers to a 60x60 graphic.
* * `sx` and `sy`: The number of columns and rows, respectively, in the sprite sheet.
* `GUI`: The in-flight GUI used when flying this ship.
* `sound`: Sound effect used when accelerating during flight.
* `class`: Defines the ship's AI when flown by escorts and NPCs.
* * `display`: Overrides the displayed "class" field in the ship stats screen.
* `price`: Credits value of the ship in its "dry" state with no outfits.
* `time_mod`: Time compression factor during normal flight. A value of `1` means the ship will fly in "real time", <1 speeds up the game and >1 slows down the game.
* `trail_generator`: Creates a particle trail during flight.
* * `x` and `y`: Trail origin coordinates, relative to the ship sprite in a "90 degree" heading.
* * `h`: Trail coordinate y-offset, used to modify the origin point on "perspective" graphics.
* `fabricator`: Flavor text stating the ship's manufacturuer.
* `license`: License-type outfit(s) which must be owned to purchase the ship.
* `description`: Flavor text describing the ship and its capabilities.
* `health`: Supercategory which defines the ship's intrinsic durability before modifiers from `stats` and equipped outfits.
* * `armour`: Armour value.
* * `armour_regen`: Armour regeneration in MW (MJ per second).
* * `shield`: Shield value.
* * `shield_regen`: Shield regeneration in MW (MJ per second).
* * `energy`: Energy capacity.
* * `energy_regen`: Energy regeneration in MW (MJ per second).
* * `absorb`: Reduction to incoming damage.
* `characteristics`: Additional ship stats involving integers.
* * `crew`: Number of crewmen operating the ship. Used in boarding actions.
* * `mass`: Tonnage of the ship hull without any cargo or outfits.
* * `fuel_consumption`: How many units of fuel the ship consumes to make a hyperspace jump.
* * `cargo`: Capacity for tonnes of cargo.
* `slots`: Outfit slots.
* * `weapon`, `utility` and `structure`: Defines whether the outfit slot fits under the Weapon, Utility or Structure columns.
* * * `x`, `y` and `h` define the origin coordinates of weapon graphics such as projectiles, particles and launched fighters.
* * * `size`: Defines the largest size of outfit allowed in the slot. Valid values are `small`, `medium` and `large`.
* * * `prop`: Defines the slot as accepting a particular type of outfit. Of note are the `systems`, `engines` and `hull` values for Core Systems, Engines and Hull outfits which must be filled for a ship to be spaceworthy.
* * * `exclusive=1`: Restricts the slot to accepting only the outfits defined by the `prop` field.
* * * Inserting an outfit's `name` will add it to that outfit slot in the ship's "stock" configuration. This is useful for selling a ship with prefilled core outfits to ensure its spaceworthiness immediately upon purchase.
* `stats`: Defines modifiers applied to all characteristics and outfits on the ship.
* * Fields here correspond to those in the `characteristics` category and the `general` and `specifics` categories on equipped outfits.
* `tags`: Referenced by scripts. Can be used to effect availability of missions, NPC behavior and other elements.
