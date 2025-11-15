# Plugin Meta-Data `plugin.toml`

The `plugin.toml` file is specific to plugins and does not exist in the base game. A full example with only the required fields is shown below:

```toml
identifier = "MyPlugin" # Has to be unique
name = "My Awesome Plugin"
author = "Me, Myself, and I"
version = "1.0.0"
abstract = "The coolest plugin on the web."
naev_version = ">= 0.13.0"
source = { git = "https://my.awesome.website/plugin/repo.git" }
```

## Required Fields

* **identifier**: has to be a unique identifier of the plugin, as plugins with the same identifier will overwrite each other.
  Only ASCII alphanumeric (letters from a-z both uppercase and lowercase, and numbers) are allowed, and it must not exceed 25 characters.
* **name**: attribute that contains the name of the plugin.
  This is what the player and other plugins will see when searching or installing the plugin.
* **author**: contains the name of the author(s) of the plugin.
* **version**: contains the version of the plugin.
  This has to be a [semver](https://semver.org) string, that is usually in the form of "X.Y.Z" where X is the major version, Y is the major version, and Z is the minor version.
* **abstract**: contains the abstract of the plugin.
  This is required and must not exceed 200 characters.
  It is what players see when searching.
* **naev_version**: specifies which verisons of naev are compatible.
  This is based on [semver](https://semver.org/) and allows comparisons with existing versions.
  For example, `>= 0.13` would mean a version after 0.13.0 (including betas), while something like `>= 0.13, < 0.14` would specify something newer than `0.13.0` but older than `0.14.0`.
* **source**: defines where the source is located to try to update the plugin when possible.
  It can be set to `source = "local"` to not search online.
  Other valid options are `source = { git = "url" }` or `source = { download = "url" }` depending on whether it is a direct download or a git repository.

## Optional Fields

* **description**: contains the description of the plugin.
  This can be as long as necessary.
  In the case it is not defined, the **abstract** field will be used in its place.
* **license**: a free-form string defining the license.
  Usually abbreviated license names such as "GPLv3+", "CC0" or "CC-by-sa 3.0" are used.
* **release_status**: defines the status of the plugin.
  This can be one of "stable", "testing", or "development".
  If omitted, it defaults to "stable".
* **tags**: a list of free-form tags such as `[ "tc", "casual" ]`.
* **image_url**: the location of a representative image of the plugin.
  This will be shown in the plugin manager and cached.
  The url must have an explicit extension such as `.webp` or `.png`.
* **depends**: a list of plugin identifiers that this plugin depends on such as `[ "SomeOtherPlugin", "YetAnotherPlugin" ]`.
* **recommends** a list of plugin identifiers that this plugin recommends usage jointly with.
* **priority**: indicates the loading order of the plugin.
  The default value is 5 and a lower value indicates higher priority.
  Higher priority allows the plugin to overwrite files of lower priority plugins.
  For example, if two plugins have a file `missions/test.lua`, the one with the lower priority would take preference and overwrite the file of the other plugin.
* **total_conversion**: a boolean value whether this plugin is a total conversion.
* **blacklist**: a list of regex strings representing files to blacklist as explained below.
* **whitelist** a list of regex strings representing files to whitelist as explained below.

## Blacklisting and Whitelisting Base Files

Furthermore, it is also possible to use regex to hide files from the base game with the `blacklist` field.
For example, `blacklist = [ "^ssys/.*\.xml" ]` would hide all the XML files in the `ssys` directory.
This is especially useful when planning total conversions or plugins that modify significantly the base game, and you don't want updates to add new files you have to hide.
By using the appropriate blacklists, you increase compatibility with future versions of Naev.
Furthermore, given the popularity of *total conversion*-type plugins, you can use the `total_conversion = true` tag to apply a large set of blacklist rules which will remove all explicit content from Naev.
This means you will have to define at least a star system, a spob, and a flyable ship for the game to run.

In additiona to the blacklist, a whitelist can also be defined with `whitelist`, which takes priority over the blacklist.
In other words, whitlelist stuff will ignore the blacklist.
*Total conversions* automatically get a few critical files such as the `settings.lua` event included, although they can still be overwritten.
