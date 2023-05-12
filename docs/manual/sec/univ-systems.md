## Systems

In the context of Naev, "systems" refer to star systems, the instanced locations where starship flight and combat take place. The contents of systems consist mainly of three object types: spobs (space objects) which represent planets, space stations or other bodies of interest; asteroid fields which act as commodity sources and obstacles to weapons fire; and jump points to facilitate travel to other systems. Many systems may also have persistent effects related to nebulae including visuals, sensor interference and even constant damage over time. A "total conversion" plugin must contain at least one system to have minimum viable content.

### Universe Editor

Naev includes an in game editor to generate and modify both systems and their contents. The editor is accessible from the game's main menu when Dev Mode is enabled by either of two methods:
1) Use the `--devmode` launch option.
2) In your `conf.lua`, find and set `devmode = true`.

The universe editor is far easier to use than direct editing of .XML files. You can quickly place new systems and drag them around the map, link systems by generating jump lanes and automatically generating entry and exit points, and create spobs, virtual spobs and asteroid fields within systems.

### System XML

Each system is represented by a standalone .XML file within the `/ssys/` directory of your main or plugin data directory.

* `<ssys>`: Category which encapsulates the system's data file.
  * `<name>`: Name of the system. Use this string when referencing this system in other .XML files. This name will also be displayed within the game itself.
  * `<general>`: Includes data defining the size and traits of the system.
    * `<radius>`: Defines the physical dimensions of the system. This value is visualized in game by the scaling of the system travel map, and in the universe editor by a circle seen when editing systems. Jump points with the `<autopos/>` tag will be placed on this circle. System content such as spobs can be placed outside this radius but may be difficult for players to locate or access.
    * `<spacedust>`: Defines the density of space dust displayed in the system.
    * `<interference>`: Influences the sensors of ships in the system. A value greater than 0 will reduce the ranges at which you can detect, identify or destealth other ships. Reduction of detection, evasion, and stealth ranges is computed by the formula $\frac{1}{1+\frac{\text{interference}}{100}}$.
    * `<nebula>`: Reduces visibility when within the system. A value greater than 0 will cause ships, spobs and asteroids to not appear until the player gets close. The rough visibility range is computed from the formula $(1200-\text{nebula}) \cdot \text{ewdetect} + \text{nebuvisibility}$, where ewdetect and nebuvisibility are each ships detection and nebula visibility statistics.
      * `<volatility>`: Damage over time inflicted upon ships travelling in this system. Value is expressed in MJ per second, applied to shields first and armor after.
    * `<features>`: A string value defining unique characteristics of the system, such as whether it has a factional homeworld or some other anomality. This is shown in the in-game map.
	* `<pos>`: Position of the system on the universe map, expressed as `x` and `y` coordinates relative to the universe map's origin point.
  * `<spobs>`: Category which includes all spobs, including virtual spobs, which are present in this system.
    * `<spob>`: Adds the spob of that name to the system. The coordinate position of the spob is defined within that spob's .XML file.
    * `<spob_virtual>`: Adds the virtual spob of that name to the system. Virtual spobs are used primarily for faction presences within the system.
  * `<jumps>`: Category which includes coordinates and tags for jump points which allow players to travel to other systems.
    * `<jump>`: Defines a jump point.
      * `<target>`: Name of the jump point's destination system. The direction of travel when entering this jump point corresponds to that of the jump line shown on the universe map.
      * `<pos>`: Position of the jump point within the system, expressed as `x` and `y` coordinates relative to the system's `x="0" y="0"` origin point.
      * `<autopos/>`: Alternative to `<pos>` which prompts the game to generate a position for the jump point. The point will always be placed at the system boundary (the circle defined by <radius>) on a line between the current system center and the destination system.
        * `<exitonly/>: Prevents the player from detecting this jump point or entering it from the current system. These points are used exclusively as the destinations to jumps coming in from other systems.
      * `<hide>`: Modifies the range at which your sensors can discover previously unknown jump points. A value of `1` is the default and indicates no change. Values greater than `1` increase the jump point's detection distance. Values less than `1` but greater than `0` reduce the jump point's detection distance. A value of `0` is a specific exception which labels the jump as part of a Trade Route - the jump point will automatically be discovered when the player enters the system, regardless of distance, and also have some small beacons next to it.
      * `<hidden/>`: Designates the jump as a hidden point which cannot be discovered with standard sensors. In the base Naev scenario, hidden jump points are revealed to the player mainly via mission rewards, by completing certain missions or by equipping and activating a Hidden Jump Scanner outfit.
  * `<asteroids>`: Category which includes coordinates and contents of asteroid fields.
    * `<asteroid>`: Defines an asteroid field.
      * `<group>`: Names an .XML list from `/asteroids/groups` that defines what asteroids spawn in this field.
      * `<pos>`: Center position of the asteroid field within the system, expressed as `x` and `y` coordinates relative to the system's center point.
      * `<radius>`: Size of the circular asteroid field, expressed in distance units from the field's center point as defined in the `<pos>` field.
      * `<density>`: Affects how many asteroids are present within the asteroid field's area.
    * `<exclusion>`: Defines an asteroid exclusion zone, creating a "negative" asteroid field. This can be used to create asteroid fields of unique shapes such as rings or crescents.
      * `<radius>` and <pos> fields function identically to those under <asteroid>.

### System Tags \naev

TODO

### Defining Jumps

Within Naev, jump points are used to travel between systems. Each jump point has a position within the system, defined either manually using the `<pos>` tag and `x` and `y` values or by using the `<autopos/>` tag to automatically place the point at a distance defined by the system's `<radius>`. Jump points also have an entry vector, or direction which ships must be facing to begin a jump. This entry vector is dictated by the position of the destination system on the universe map relative to the current system - that is, a jump point will always point towards its destination system.

To create a standard two-way jump lane between two systems:
1) Within current system *a*, create a `<jump>`. Use the `<target>` tag to name destination system *b*. Use the `<autopos/>` tag to automatically place the jump point, or the `<pos>` tag to manually define its position with X and Y values.
2) Repeat the above in system *b* to create a jump point, using the `<target>` tag to name destination system *a*.

### Asteroid Fields

Asteroid fields are zones of floating objects within systems. They differ from spobs in that they are defined as circular areas rather than single points with graphics. Asteroids also interact with ship weapons fire and often generate commodity pickups when destroyed.

Asteroid data files are found in `/asteroids/types/`. These files are in .XML format and contain the following fields:
* `<scanned>`: Text string shown to the player upon entering range of their asteroid scanner outfit.
* `<gfx>`: Possible graphics for this asteroid. Multiple graphics can be referenced, one per `<gfx>` tag, to increase the variety of visuals.
* `<armor_min>` and `<armor_max>`: Defines a range of armor values for asteroids to spawn with. Higher values mean more damage must be dealt to destroy an asteroid.
* `<absorb>`: Defines the asteroid's damage reduction before applying weapons' armor penetration stats.
* `<commodity>`: Lists which commodity pickups and quantities thereof can spawn upon destruction of the asteroid.
	* `<name>`: Name of commodity.
	* `<quantity>`: Maximum quantity of commodity pickups

This process will let you create an asteroid field in your `<ssys>` .XML file:
1) Place graphics for your asteroids, in .WEBP format, to `/gfx/spob/space/asteroid/`;
2) Write asteroid data files, in .XML format, to `/asteroids/types/`;
3) Write an asteroid group list, in .XML format, to `/asteroids/groups/`.
4) In your `<ssys>` .XML file, use the `<asteroid>` field and subfields above to tell the game what asteroids the field will be made of. `<pos>` and `<radius>` define the position and size of your field. `<group>` and `<density>` define which asteroid group and how many asteroids appear in your field.
