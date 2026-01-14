# Plugin Framework

Plugins are user-made collections of files that can add or change content from Naev.
They can be seen as a set of files that can overwrite core Naev files and add new content such as missions, outfits, ships, etc.
They are implemented with [PHYSFS](https://icculus.org/physfs/) that allows treating the plugins and Naev data as a single "combined" virtual filesystems.
Effectively, Naev will see plugin files as part of core data files, and use them appropriately.

Plugins are found at the following locations by default, and are automatically loaded if found.

| Operating System | Data Location |
| --- | --- |
| Linux | `~/.local/share/naev/plugins` |
| Mac OS X | `~/Library/Application Support/org.naev.Naev/plugins` |
| Windows | `%APPDATA%\naev\plugins` |

You can also see the path that Naev is looking for plugins from the plugins tab of the options menu.

## Creating a Plugin

Plugins can either be a directory structure or compressed into a single `zip` file which allows for easier distribution.
You can create them by simply creating a directory with a [plugin.toml metadata file](./metadata.md), and it should already be usable, although it will do nothing.
To add functionality to the plugin, you have to add data files for the functionality you want.
See the information on the [data structure](../data.md) for how it is organized, and refer to the other sections for adding or modifying particular functionality.

## Plugin Repository

Since version 0.13.0, Naev has a built-in plugin explorer and installer, which uses the [plugin repository](https://codeberg.org/naev/naev-plugins) by default.
This repository contains only the minimum information of the plugins necessary to be able to download and look up the rest of the information.
To add your plugin, please create a pull request on the repository adding the following information as a `.toml` file in `plugins/`:

```toml
identifier = "MyId"
name = "My Plugin Full Name"
source = { git = "https://codeberg.org/naev/my_plugin" }
metadata = "https://codeberg.org/naev/my_plugin/raw/branch/main/plugin.toml"
```

Where:
* **identifier**: has to be a unique identifier of the plugin, as plugins with the same identifier will overwrite each other.
  Only ASCII alphanumeric (letters from a-z both uppercase and lowercase, and numbers) are allowed, and it must not exceed 25 characters.
* **name**: attribute that contains the name of the plugin.
  This is what the player and other plugins will see when searching or installing the plugin.
* **source**: defines where the source is located to try to update the plugin when possible.
  Valid options are `source = { git = "url" }` or `source = { download = "url" }` depending on whether it is a direct download or a git repository.
* **metadata**: should be a url pointing to where the plugin's `plugin.toml` is available.
  The plugin manager will use this file to see if updates are necessary.
