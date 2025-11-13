# Data Structure

Naev plugins and data use the same directory structure.
It is best to open up the original data to see how everything is laid.
For completeness, the main directories are described below:

* `ai/`: contains the different [AI profiles](./ai/overview.md) and their associated libraries.
* `asteroids/`: contains the different asteroid types and groups in different directories.
* `commodities/`: contains all the different commodity files.
* `damagetype/`: contains all the potential damage types.
* `difficulty/`: contains the different difficulty settings.
* `effects/`: contains information about effects that can affect ships.
* `events/`: contains all the [events](./misn/overview.md).
* `factions/`: contains all the factions and their related Lua functionality.
* `glsl/`: contains all the shaders. Some are critical for basic game functionality.
* `gui/`: contains the different GUIs
* `map_decorator/`: contains the information of what images to render on the map.
* `missions/`: contains all the [missions](./misn/overview.md).
* `outfits/`: contains all the [outfits](./outfits/overview.md).
* `scripts/`: this is an optional directory that contains all libraries and useful scripts by convention. It is directly appended to the Lua path, so you can `require` files in this directory directly without having to prepend `scripts.`.
* `ships/`: contains all the [ships](./ships/overview.md).
* `slots/`: contains information about the different ship slot types.
* `snd/`: contains all the sound and music.
* `spfx/`: contains all the special effects. Explosions are required by the engine and can not be removed.
* `spob/`: contains all the [space objects](./univ/spobs.md) (planets, stations, etc.).
* `spob_virtual/`: contains all the virtual space objects. These mainly serve to modify the presence levels of factions in different systems artificially.
* `ssys/`: contains all the [star systems](./univ/systems.md).
* `tech/`: contains all the tech structures.
* `trails/`: contains all the descriptions of ship trails that are available and their shaders.
* `unidiff/`: contains all the universe diffs. These are used to create modifications to the game data during a playthrough, such as adding spobs or systems.

In general, recursive iteration is used with all directories.
This means you don't have to put all the ship XML files directly it `ships/`, but you can use subdirectories, such as `ships/special/`.
Furthermore, in order to avoid collision between plugins, it is highly recommended to use a subdirectory with the plugin name.
So if you want to define a new ship called `Stardragon`, you would put the XML file in `ships/my_plugin/stardragon.xml`.

Furthermore, the following files play a critical role:

* `AUTHORS`: contains author information about the game.
* `VERSION`: contains version information about the game.
* `autoequip.lua`: used when the player presses autoequip in the equipment window.
* `board.lua`: used when the player boards a ship.
* `comm.lua`: used when the player hails a ship.
* `common.lua`: changes to the Lua language that are applied to all scripts.
* `intro`: the introduction text when starting a new game.
* `loadscreen.lua`: renders the loading screen.
* `rep.lua`: internal file for the console. Do not modify!!
* `rescue.lua`: script run when the game detects the player is stranded, such as they have a non-spaceworthy ship and are stuck in an uninhabited spob.
* `save_updater.lua`: used when updating saves to replace old outfits and licenses with newer ones.
* `start.xml`: determines the starting setting, time and such of the game.
