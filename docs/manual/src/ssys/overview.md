# Star Systems (Ssys)

In the context of Naev, "systems" refer to star systems, the instanced locations where starship flight and combat take place.
The contents of systems consist mainly of three object types: spobs (space objects) which represent planets, space stations or other bodies of interest; asteroid fields which act as commodity sources and obstacles to weapons fire; and jump points to facilitate travel to other systems.
Many systems may also have persistent effects related to nebulae including visuals, sensor interference and even constant damage over time.
A "total conversion" plugin must contain at least one system to have minimum viable content.

Each system can contain multiple [Space Objects](../spobs/overview.md) that can represent planets, space stations, wormholes, and other objects the player can usually interact with.
They also contain jumps to other adjacent systems.

## Universe Editor

Naev includes an in game editor to generate and modify both systems and their contents.
The editor is accessible from the game's main menu through the "Extras" menu.

The universe editor is far easier to use than direct editing of `XML` files.
You can quickly place new systems and drag them around the map, link systems by generating jump lanes and automatically generating entry and exit points, and create spobs, virtual spobs and asteroid fields within systems.
