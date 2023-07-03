# Plugin Framework
\label{chapter:plugins}

Plugins are user-made collections of files that can add or change content from Naev. They can be seen as a set of files that can overwrite core Naev files and add new content such as missions, outfits, ships, etc. They are implemented with [PHYSFS](https://icculus.org/physfs/) that allows treating the plugins and Naev data as a single "combined" virtual filesystems. Effectively, Naev will see plugin files as part of core data files, and use them appropriately.

Plugins are found at the following locations by default, and are automatically loaded if found.

| Operating System | Data Location |
| --- | --- |
| Linux | `~/.local/share/naev/plugins` |
| Mac OS X |  `~/Library/Application Support/org.naev.Naev/plugins` |
| Windows | `\%APPDATA\%\naev\plugins` |

Plugins can either be a directory structure or compressed into a single `zip` file which allows for easier distribution.

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

Finally, plugins have access to an additional important file known as `plugin.xml` that stores meta-data about the plugin itself and compatibility with Naev versions. This is explained in the next section.

## Plugin Meta-Data `plugin.xml`

The `plugin.xml` file is specific to plugins and does not exist in the base game. A full example is shown below:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<plugin name="My Plugin">
 <author>Me</author>
 <version>1.0</version>
 <description>A cool example plugin.</description>
 <compatibility>^0\.10\..*</compatibility>
 <priority>3</priority>
</plugin>
```

The important fields are listed below:

* `name`: attribute that contains the name of the plugin. This is what the player and other mods will see and use to reference the plugin.
* `author`: contains the name of the author(s) of the plugin.
* `version`: contains the version of the plugin. This can be any arbitrary string.
* `description`: contains the description of the plugin. This should be easy to understand for players when searching for plugins.
* `compatibility`: contains compatibility information. Specifically, it must be a regex string that will be tested against the current Naev string. Something like `^0\.10\..*` will match any version string that starts with `"0.10."`. Please refer to a regular expression guide such as [regexr](https://regexr.com/) for more information on how to write regex.
* `priority`: indicates the loading order of the plugin. The default value is 5 and a lower value indicates higher priority. Higher priority allows the plugin to overwrite files of lower priority plugins. For example, if two plugins have a file `missions/test.lua`, the one with the lower priority would take preference and overwrite the file of the other plugin.

Furthermore, it is also possible to use regex to hide files from the base game with `<blacklist>` nodes. For example, `<blacklist>^ssys/.*\.xml</blacklist>` would hide all the XML files in the `ssys` directory. This is especially useful when planning total conversions or plugins that modify significantly the base game, and you don't want updates to add new files you have to hide. By using the appropriate blacklists, you increase compatibility with future versions of Naev. Furthermore, given the popularity of *total conversion*-type plugins, you can use the `<total_conversion/>` tag to apply a large set of blacklist rules which will remove all explicit content from Naev. This means you will have to define at least a star system, a spob, and a flyable ship for the game to run.

## Plugin Repository

Naev has a [plugin repository](https://github.com/naev/naev-plugins) which tries to centralize know plugins. To add your plugin, please create a pull request on the repository. This repository contains only the minimum information of the plugins necessary to be able to download and look up the rest of the information.
