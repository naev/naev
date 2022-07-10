# Introduction

Welcome to the Naev development manual! This manual is meant to cover all aspects of Naev development. It is currently a work in progress.

While this document does cover the Naev engine in general, many sections refer to customs and properties specific to the **Sea of Darkness** default Naev universe. These are marked with \naev.

## Getting Started

This document assumes you have access to the Naev data. This can be either from downloading the game directly from a distribution platform, or getting directly the [naev source code](https://github.com/naev/naev). Either way it is possible to modify the game data and change many aspects of the game. It is also possible to create plugins that add or replace content from the game without touching the core data to be compatible with updates.

| Operating System | Data Location |
| --- | --- |
| Linux | `/usr/share/naev/dat` |
| Mac OS X | `/Applications/Naev.app/Contents/Resources/dat` |
| Windows | TODO |

Most changes will only take place when you restart Naev, although it is possible to force Naev to reload a mission or event with `naev.missionReload` or `naev.eventReload`.

## Plugins

Naev supports arbitrary plugins. These are implemented with a virtual filesystem based on [PHYSFS](https://icculus.org/physfs/). The plugin files are therefore "combined" with existing files in the virtual filesystem, with plugin files taking priority. So if you add a mission in a plugin, it gets added to the pool of available missions. However, if the mission file has the same name as an existing mission, it will overwrite it. This allows the plugin to change core features such as boarding or communication mechanics or simply add more compatible content.

Plugins are found at the following locations by default, and are automatically loaded if found.

| Operating System | Data Location |
| --- | --- |
| Linux | `~/.local/share/naev/plugins` |
| Mac OS X |  `~/Library/Application Support/org.naev.Naev/plugins` |
| Windows | `%APPDATA%\naev\plugins` |

Note that plugins can use either a directory structure or be compressed as zip files (while still having the appropriate directory structure). For example, it is possible to add a single mission by creating a plugin with the follow structure:

```
missions/
   my_mission.xml
```

This will cause `my_mission.xml` to be loaded as an extra mission.

## Directory Structure

Naev plugins and data use the same directory structure. It is best to open up the original data to see how everything is laid. For completeness, the main directories are described below:

* `ai/`: contains the different AI profiles and their associated libraries.
* `asteroids/`: contains the different asteroid types and groups in different directories.
* `commodities/`: contains all the different commodity files.
* `damagetype/`: contains all the potential damage types.
* `difficulty/`: contains the different difficulty settings.
* `effects/`: contains information about effects that can affect ships.
* `events/`: contains all the events.
* `factions/`: contains all the factions and their related Lua functionality.
* `glsl/`: contains all the shaders. Some are critical for basic game functionality.
* `gui/`: contains the different GUIs
* `map_decorator/`: contains the information of what images to render on the map.
* `missions/`: contains all the missions.
* `outfits/`: contains all the outfits.
* `scripts/`: this is an optional directory that contains all libraries and useful scripts by convention. It is directly appended to the Lua path, so you can `require` files in this directory directly without having to prepend `scripts.`.
* `ships/`: contains all the ships.
* `slots/`: contains information about the different ship slot types.
* `snd/`: contains all the sound and music.
* `spfx/`: contains all the special effects. Explosions are required by the engine and can not be removed.
* `spob/`: contains all the space objects (planets, stations, etc.).
* `spob_virtual/`: contains all the virtual space objects. These mainly serve to modify the presence levels of factions in different systems artificially.
* `ssys/`: contains all the star systems.
* `tech/`: contains all the tech structures.
* `trails/`: contains all the descriptions of ship trails that are available and their shaders.
* `unidiff/`: contains all the universe diffs. These are used to create modifications to the game data during a playthrough, such as adding spobs or systems.

In general, recursive iteration is used with all directories. This means you don't have to put all the ship xml files directly it `ships/`, but you can use subdirectories. Furthermore, in order to avoid collision between plugins, it is highly recommended to use a subdirectory with the plugin name. So if you want to define a new ship called `Stardragon`, you would put the xml file in `ships/my_plugin/stardragon.xml`.

Furthermore, the following files play a critical role:

* `AUTHORS`: contains author information about the game. Should contain only information about the plugin.
* `VERSION`: contains version information about the game. Should contain only information about the plugin.
* `autoequip.lua`: used when the player presses autoequip in the equipment window.
* `board.lua`: used when the player boards a ship.
* `comm.lua`: used when the player hails a ship.
* `common.lua`: changes to the Lua language that are applied to all scripts.
* `intro`: the introduction text when starting a new game.
* `landing.lua`: handles how spobs allow the player to land. TODO rewrite.
* `loadscreen.lua`: renders the loading screen.
* `rep.lua`: internal file for the console. Do not modify!!
* `rescue.lua`: script run when the game detects the player is stranded, such as they have a non-spaceworthy ship and are stuck in an uninhabited spob.
* `save_updater.lua`: used when updating saves to replace old outfits and licenses with newer ones.
* `start.xml`: determines the starting setting, time and such of the game.

Note that there is still a significant amount of monolithic files that need to be replaced with directories. Editing monolithic files will be bad for future save compatibility.
