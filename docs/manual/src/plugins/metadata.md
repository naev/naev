## Plugin Meta-Data `plugin.xml`

The `plugin.xml` file is specific to plugins and does not exist in the base game. A full example is shown below:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<plugin name="My Plugin">
 <author>Me</author>
 <version>1.0</version>
 <description>A cool example plugin.</description>
 <naev_version>&gt;= 0.13.0</naev_version>
 <priority>3</priority>
 <source>https://source</source>
</plugin>
```

The important fields are listed below:

* `name`: attribute that contains the name of the plugin. This is what the player and other mods will see and use to reference the plugin.
* `author`: contains the name of the author(s) of the plugin.
* `version`: contains the version of the plugin. This can be any arbitrary string.
* `description`: contains the description of the plugin. This should be easy to understand for players when searching for plugins.
* `naev_version`: specifies which verisons of naev are compatible. This is based on [semver](https://semver.org/) and allows comparisons with existing versions. For example, `>= 0.13` would mean a version after 0.13.0 (including betas), while something like `>= 0.13, < 0.14` would specify something newer than `0.13.0` but older than `0.14.0`. Please note that since this is XML, you have to escape `>` with `&gt;` and `<` with `&gt;`.
* `priority`: indicates the loading order of the plugin. The default value is 5 and a lower value indicates higher priority. Higher priority allows the plugin to overwrite files of lower priority plugins. For example, if two plugins have a file `missions/test.lua`, the one with the lower priority would take preference and overwrite the file of the other plugin.
* `source`: points to where a newer version can be obtained or downloaded. This could be a website or other location.

Furthermore, it is also possible to use regex to hide files from the base game with `<blacklist>` nodes. For example, `<blacklist>^ssys/.*\.xml</blacklist>` would hide all the XML files in the `ssys` directory. This is especially useful when planning total conversions or plugins that modify significantly the base game, and you don't want updates to add new files you have to hide. By using the appropriate blacklists, you increase compatibility with future versions of Naev. Furthermore, given the popularity of *total conversion*-type plugins, you can use the `<total_conversion/>` tag to apply a large set of blacklist rules which will remove all explicit content from Naev. This means you will have to define at least a star system, a spob, and a flyable ship for the game to run.

In additiona to the blacklist, a whitelist can also be defined with `<whitelist>`, which takes priority over the blacklist. In other words, whitlelist stuff will ignore the blacklist. *Total conversions* automatically get a few critical files such as the `settings.lua` event included, although they can still be overwritten.
