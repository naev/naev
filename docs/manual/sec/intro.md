# Introduction to the Naev Engine

While this document does cover the Naev engine in general, many sections refer to customs and properties specific to the **Sea of Darkness** default Naev universe. These are marked with \naev.

## Getting Started

Th Naev engine explanations assume you have access to the Naev data. This can be either from downloading the game directly from a distribution platform, or getting directly the [naev source code](https://github.com/naev/naev). Either way it is possible to modify the game data and change many aspects of the game. It is also possible to create plugins that add or replace content from the game without touching the core data to be compatible with updates.

| Operating System | Data Location |
| --- | --- |
| Linux | `/usr/share/naev/dat` |
| Mac OS X | `/Applications/Naev.app/Contents/Resources/dat` |
| Windows | `\%ProgramFiles(x86)\%\Naev\dat` |

Most changes will only take place when you restart Naev, although it is possible to force Naev to reload a mission or event with `naev.missionReload` or `naev.eventReload`.

## Plugins

Naev supports arbitrary plugins. These are implemented with a virtual filesystem based on [PHYSFS](https://icculus.org/physfs/). The plugin files are therefore "combined" with existing files in the virtual filesystem, with plugin files taking priority. So if you add a mission in a plugin, it gets added to the pool of available missions. However, if the mission file has the same name as an existing mission, it will overwrite it. This allows the plugin to change core features such as boarding or communication mechanics or simply add more compatible content.

Plugins are found at the following locations by default, and are automatically loaded if found.

| Operating System | Data Location |
| --- | --- |
| Linux | `~/.local/share/naev/plugins` |
| Mac OS X |  `~/Library/Application Support/org.naev.Naev/plugins` |
| Windows | `\%APPDATA\%\naev\plugins` |

Note that plugins can use either a directory structure or be compressed as zip files (while still having the appropriate directory structure). For example, it is possible to add a single mission by creating a plugin with the follow structure:

```
plugin.xml
missions/
   my_mission.xml
```

This will cause `my_mission.xml` to be loaded as an extra mission. `plugin.xml` is a plugin-specific file which would contain information on plugin name, authors, version, description, compatibility, and so on.

Plugins are described in detail in Chapter \ref{chapter:plugins}.
