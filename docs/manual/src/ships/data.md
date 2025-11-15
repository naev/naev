# Ship Data Format

Each ship is represented with a stand-alone file that has to be located in `ships/` in the data files or plugins.
Each ship has to be defined in a separate file and has to have a single `<ship>` base node.

* `name` (*attribute*): Ship name, displayed in game and referenced by `tech` lists.
* `points`: Fleet point value. In general used by both the fleet spawning code and by player fleets.
* `base_type`: Specifies the base version of the ship, useful for factional or other situational variants.
  (For example, a Pirate Hyena would have the "Hyena" base type.)
* `GFX`: Name of the ship graphic in `.webp` format.
  It is looked up at `gfx/ship/DIR/NAME`, where `DIR` is the value of `GFX` up to the first underscore, and `NAME` is the value of `GFX` with a special suffix depending on the type of image.
  The base image will try different common file suffixes if not specified.
  The comm window graphic will use a suffix of `_comm.webp`, and the engine glow will use a suffix of `_engine.webp`.
  As an example, for a value of `GFX="hyena_pirate"`, the base graphic will be searched at `gfx/ship/hyena/hyena_pirate.webp`
  * `size` (*attribute*): The ship sprite's resolution in pixels.
  For example, `size=60` refers to a 60x60 graphic.
  * `sx` and `sy` (*attributes*): The number of columns and rows, respectively, in the sprite sheet.
* `GUI`: The in-flight GUI used when flying this ship.
* `sound`: Sound effect used when accelerating during flight.
* `class`: Defines the ship's AI when flown by escorts and NPCs.
  * `display` (*attribute*): Overrides the displayed "class" field in the ship stats screen.
* `price`: Credits value of the ship in its "dry" state with no outfits.
* `time_mod` (*optional*): Time compression factor during normal flight.
  A value of `1` means the ship will fly in "real time", <1 speeds up the game and >1 slows down the game.
* `trail_generator` (unecessary with 3D ships): Creates a particle trail during flight.
  * `x`, `y` (*attributes*): Trail origin coordinates, relative to the ship sprite in a "90 degree" heading.
  * `h` (*attributes*): Trail coordinate y-offset, used to modify the origin point on a "perspective" camera.
* `fabricator`: Flavour text stating the ship's manufacturer.
* `faction` (*optional*): Defines the faction which produces the ship. Currently only used for the fancy gradient backgrounds.
* `licence` (*optional*): Licence-type outfit which must be owned to purchase the ship.
* `cond` (*optional*): Lua conditional expression to evaluate to see if the player can buy the ship.
* `condstr` (*optional*): human-readable interpretation of the Lua conditional expression `cond`.
* `description`: Flavour text describing the ship and its capabilities.
* `characteristics`: Core ship characteristics that are defined as integers.
  * `crew`: Number of crewmen operating the ship. Used in boarding actions.
  * `mass`: Tonnage of the ship hull without any cargo or outfits.
  * `fuel_consumption`: How many units of fuel the ship consumes to make a hyperspace jump.
  * `cargo`: Capacity for tonnes of cargo.
* `slots`: List of available outfit slots of the ship.
  * `weapon`, `utility` and `structure`: Defines whether the outfit slot fits under the Weapon, Utility or Structure columns.
    * `x`, `y`, and `h` (*attributes*) define the origin coordinates of weapon graphics such as projectiles, particles and launched fighters.
    * `size` (*attribute*): Defines the largest size of outfit allowed in the slot.
    Valid values are `small`, `medium` and `large`.
    * `prop` (*attribute*): Defines the slot as accepting a particular type of outfit defined by an .
    XML file in the `slots/` directory. The Naev default scenario includes `systems`, `engines`, and `hull` values for Core Systems, Engines, and Hull outfits which must be filled (if they exist) for a ship to be spaceworthy.
        * `exclusive=1` (*attribute*): Restricts the slot to accepting only the outfits defined by the `prop` field.
        * Inserting an outfit's `name` will add it to that outfit slot in the ship's "stock" configuration. This is useful for selling a ship with prefilled core outfits to ensure its spaceworthiness immediately upon purchase.
* `stats` (*optional*): Defines modifiers applied to all characteristics and outfits on the ship.
  * Fields here correspond to those in the `characteristics` category and the `general` and `specifics` categories on equipped outfits.
* `tags` (*optional*): Referenced by scripts. Can be used to effect availability of missions, NPC behaviour and other elements.
  * `tag`: Each `tag` node represents a binary flag which are accessible as a table with `ship.tags()`
* `health`: Supercategory which defines the ship's intrinsic durability before modifiers from `stats` and equipped outfits. **Note that this node and subnodes are deprecated and will likely be removed in future versions. Use ship stats instead!**
  * `armour`: Armour value.
  * `armour_regen`: Armour regeneration in MW (MJ per second).
  * `shield`: Shield value.
  * `shield_regen`: Shield regeneration in MW (MJ per second).
  * `energy`: Energy capacity.
  * `energy_regen`: Energy regeneration in MW (MJ per second).
  * `absorb`: Reduction to incoming damage.

For a full example of the $\naev$ starter ship ``Llama'' is shown below, refer to the [original file](https://codeberg.org/naev/naev/src/branch/main/dat/ships/neutral/llama.xml)
