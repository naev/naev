# Engine Constants

It is possible to change a significant number of core engine functionality by changing different constants.
These are loaded once at the start of the game and can not be changed afterwards, and are located in the `constants.lua` file ([latest version in the repo](https://codeberg.org/naev/naev/src/branch/main/dat/constants.lua)).
For details of each specific constant, please refer to the comments in the base file.

Although these are loaded by the engine, it is also possible to access them via Lua by using `require "constants.lua"`.

## Modifying Specific Constants

Although it is possible to overwrite the file, usually you only want to change a few constants, and not redefine them all, which may cause issue when new ones get added.
You can do this by defining a plugin-specific file in the `constants/` directory that will overwrite only a subset of constants.

For example, if you add a file `constants/myplugin.lua` with the following contents:
```lua
return {
   PILOT_DISABLED_ARMOUR = 0.0,
}
```
It will only change the `PILOT_DISABLED_ARMOUR` constant, and in this case, make it so pilots never get disabled when they go under a certain amount of health.
