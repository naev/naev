# Ships

Ships are tho cornerstone of gameplay in Naev. The player themselves is represented as a ship and so are all other NPCs found in space.

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

Each ship is represented with a stand alone file that has to be located in `ships/` in the data files or plugins. Each ship has to be defined in a separate file and has to have a single `<ship>` base node.

* `name` (*attribute*): Ship name, displayed in game and referenced by `tech` lists.
* `points`: Fleet point value. In general used by both the fleet spawning code and by player fleets.
* `base_type`: Specifies the base version of the ship, useful for factional or other situational variants. (For example, a Pirate Hyena would have the "Hyena" base type.
* `GFX`: Name of the ship graphic in `.webp` format. It is looked up at `gfx/ship/DIR/NAME`, where `DIR` is the value of `GFX` up to the first underscore, and `NAME` is the value of `GFX` with a special suffix depending on the type of image. The base image will use a suffix of `.webp` (or `.png` if the webp is not found), the comm window graphic will use a suffix of `_comm.webp`, and the engine glow will use a suffix of `_engine.webp`. As an example, for a value of `GFX="hyena_pirate`, the base graphic will be searched at `gfx/ship/hyena/hyena_pirate.webp`
    * `size` (*attribute*): The ship sprite's resolution in pixels. For example, `size=60` refers to a 60x60 graphic.
    * `sx` and `sy` (*attributes*): The number of columns and rows, respectively, in the sprite sheet.
* `GUI`: The in-flight GUI used when flying this ship.
* `sound`: Sound effect used when accelerating during flight.
* `class`: Defines the ship's AI when flown by escorts and NPCs.
    * `display` (*attribute*): Overrides the displayed "class" field in the ship stats screen.
* `price`: Credits value of the ship in its "dry" state with no outfits.
* `time_mod` (*optional*): Time compression factor during normal flight. A value of `1` means the ship will fly in "real time", <1 speeds up the game and >1 slows down the game.
* `trail_generator`: Creates a particle trail during flight.
    * `x`, `y` (*attributes*): Trail origin coordinates, relative to the ship sprite in a "90 degree" heading.
    * `h` (*attributes*): Trail coordinate y-offset, used to modify the origin point on a "perspective" camera.
* `fabricator`: Flavor text stating the ship's manufacturer.
* `license` (*optional*): License-type outfit which must be owned to purchase the ship.
* `cond` (*optional*): Lua conditional expression to evaluate to see if the player can buy the ship.
* `condstr` (*optional*): human-readable interpretation of the Lua conditional expression `cond`.
* `description`: Flavor text describing the ship and its capabilities.
* `characteristics`: Core ship characteristics that are defined as integers.
    * `crew`: Number of crewmen operating the ship. Used in boarding actions.
    * `mass`: Tonnage of the ship hull without any cargo or outfits.
    * `fuel_consumption`: How many units of fuel the ship consumes to make a hyperspace jump.
    * `cargo`: Capacity for tonnes of cargo.
* `slots`: List of available outfit slots of the ship.
    * `weapon`, `utility` and `structure`: Defines whether the outfit slot fits under the Weapon, Utility or Structure columns.
        * `x`, `y`, and `h` (*attributes*) define the origin coordinates of weapon graphics such as projectiles, particles and launched fighters.
        * `size` (*attribute*): Defines the largest size of outfit allowed in the slot. Valid values are `small`, `medium` and `large`.
        * `prop` (*attribute*): Defines the slot as accepting a particular type of outfit defined by an .XML file in the `slots` folder. The Naev default scenario includes `systems`, `engines` and `hull` values for Core Systems, Engines and Hull outfits which must be filled for a ship to be spaceworthy.
        * `exclusive=1` (*attribute*): Restricts the slot to accepting only the outfits defined by the `prop` field.
        * Inserting an outfit's `name` will add it to that outfit slot in the ship's "stock" configuration. This is useful for selling a ship with prefilled core outfits to ensure its spaceworthiness immediately upon purchase.
* `stats` (*optional*): Defines modifiers applied to all characteristics and outfits on the ship.
    * Fields here correspond to those in the `characteristics` category and the `general` and `specifics` categories on equipped outfits.
* `tags` (*optional*): Referenced by scripts. Can be used to effect availability of missions, NPC behavior and other elements.
    * `tag`: Each `tag` node represents a binary flag which are accessible as a table with `ship.tags()`
* `health`: Supercategory which defines the ship's intrinsic durability before modifiers from `stats` and equipped outfits. **Note that this node and subnodes are deprecated and will likely be removed in future versions. Use ship stats instead!**
    * `armour`: Armour value.
    * `armour_regen`: Armour regeneration in MW (MJ per second).
    * `shield`: Shield value.
    * `shield_regen`: Shield regeneration in MW (MJ per second).
    * `energy`: Energy capacity.
    * `energy_regen`: Energy regeneration in MW (MJ per second).
    * `absorb`: Reduction to incoming damage.

A full example of the \naev starter ship "Llama" is shown below.

```xml
<?xml version='1.0' encoding='UTF-8'?>
<ship name="Llama">
 <points>20</points>
 <base_type>Llama</base_type>
 <GFX size="47">llama</GFX>
 <GUI>brushed</GUI>
 <sound>engine</sound>
 <class>Yacht</class>
 <price>120000</price>
 <time_mod>1</time_mod>
 <trail_generator x="-12" y="-16" h="-2">nebula</trail_generator>
 <trail_generator x="-12" y="16" h="-2">nebula</trail_generator>
 <trail_generator x="-12" y="-6" h="0">fire-thin</trail_generator>
 <trail_generator x="-12" y="0" h="0">fire-thin</trail_generator>
 <trail_generator x="-12" y="6" h="0">fire-thin</trail_generator>
 <fabricator>Melendez Corp.</fabricator>
 <description>One of the most widely used ships in the galaxy. Renowned for its stability and stubbornness. The design has not been modified much since its creation many, many cycles ago. It was one of the first civilian use spacecrafts, first used by aristocracy and now used by everyone who cannot afford better.</description>
 <characteristics>
  <crew>2</crew>
  <mass>80</mass>
  <fuel_consumption>100</fuel_consumption>
  <cargo>15</cargo>
 </characteristics>
 <slots>
  <weapon size="small" x="7" y="0" h="1" />
  <weapon size="small" x="-3" y="0" h="2" />
  <utility size="small" prop="systems">Unicorp PT-16 Core System</utility>
  <utility size="small" prop="accessory" />
  <utility size="small" />
  <utility size="small" />
  <structure size="small" prop="engines">Nexus Dart 150 Engine</structure>
  <structure size="small" prop="hull">Unicorp D-2 Light Plating</structure>
  <structure size="small" />
  <structure size="small" />
 </slots>
 <stats>
  <armour>25<armour>
  <speed_mod>-10</speed_mod>
  <turn_mod>-10</turn_mod>
  <cargo_mod>20</cargo_mod>
  <armour_mod>10</armour_mod>
  <cargo_inertia>-20</cargo_inertia>
  <ew_hide>-10</ew_hide>
 </stats>
 <tags>
  <tag>standard</tag>
  <tag>transport</tag>
 </tags>
</ship>
```

## Ship Graphics

Ship graphics are defined in the `<GFX>` node as a string with additional attributes like number of sprites or size also defined in the XML. Graphics for each ship are stored in a directory found in `gfx/ship/`, where the base graphics, engine glow graphics, and comm window graphics are placed separately with specific file names.

In particular, the `GFX` string name is sensitive to underscores, and the first component up to the first underscore is used as the directory name. As an example, with `<GFX>llama</GFX>`, the graphics would have to be put in `gfx/ship/llama/`, while for `<GFX>hyena_pirate</GFX>`, the directory would be `gfx/ship/hyena`. The specific graphics are then searched for inside the directory with the full `GFX` string value and a specific prefix. Assuming `GFX` is the graphics name and `DIR` is the directory name (up to first underscore in `GFX`), we get:

* `gfx/ship/DIR/GFX.webp`: ship base graphic file
* `gfx/ship/DIR/GFX_engine.webp`: ship engine glow graphics file
* `gfx/ship/DIR/GFX_comm.webp`: ship communication graphics (used in the comm window)

The base graphics are stored as a spritesheet and start facing right before spinning counter-clockwise. The top-left sprite faces to the right and it rotates across the row first before going down to the next row. The background should be stored in RGBA with a transparent background. An example can be seen in Figure \ref{fig:llamagfx}.

\begin{figure}[h!]
\centering
\colorbox{black}{\includegraphics[width=0.8\linewidth]{images/llama.png}}
\caption{Example of the ship graphics for the "Llama". Starting from top-left position, and going right first before going down, the ship rotates counter-clockwise and starts facing right. A black background has been added for visibility.}
\label{fig:llamagfx}
\end{figure}

The engine glow graphics are similar to the base graphics, but should show engine glow of the ship. This graphic gets used instead of the normal graphic when accelerated with some interpolation to fade on and off. An example is shown in Figure \ref{fig:llamaenginegfx}.

\begin{figure}[h!]
\centering
\colorbox{black}{\includegraphics[width=0.8\linewidth]{images/llamaengine.png}}
\caption{Example of the engine glow graphics for the "Llama". Notice the yellow glow of the engines. A black background has been added for visibility.}
\label{fig:llamaenginegfx}
\end{figure}

The comm graphics should show the ship facing the player and be higher resolution. This image will be shown in large when the player communicates with them. An example is shown in Figure \ref{fig:llamacommgfx}.

\begin{figure}[h!]
\centering
\includegraphics[width=0.8\linewidth]{images/llamacomm.png}
\caption{Example of the comm graphics for the "Llama".}
\label{fig:llamacommgfx}
\end{figure}

### Specifying Full Paths

It is also possible to avoid all the path logic in the `<GFX>` nodes by specifying the graphics individually using other nodes. In particular, you can use the following nodes in the XML in place of a single `<GFX>` node to specify graphics:

* `<gfx_space>`: Indicates the full path to the base graphics (`gfx/` is prepended). The `sx` and `sy` attributes should be specified or they default to 8.
* `<gfx_engine>`: Indicates the full path to the engine glow graphics (`gfx/` is prepended). The `sx` and `sy` attributes should be specified or they default to 8.
* `<gfx_comm>`: Indicates the full path to the comm graphics (`gfx/` is prepended).

This gives more flexibility and allows using, for example, spob station graphics for a "ship".

## Ship Conditional Expressions

TODO

## Ship trails

TODO

## Ship Slots

TODO
