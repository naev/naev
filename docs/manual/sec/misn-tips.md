## Tips and Tricks

This section contains some tricks and tips for better understanding how to do specific things in missions and events.

### Optimizing Loading

It is important to understand how missions and events are loaded. The headers are parsed at the beginning of the game and stored in memory. Whenever a trigger (entering a system, landing on a spob, etc.) happens, the game runs through all the missions and events to check to see if they should be started. The execution is done in the following way:

1. Header statements are checked (*e.g.*, unique missions that are already done are discarded)
1. Lua conditional code is compiled and run
1. Lua code is compiled and run
1. `create` function is run

In general, since many events and missions can be checked at every frame, it is important to try to cull them as soon as possible. When you can, use location or faction filters to avoid having missions and events appear in triggers you don't wish them to. In the case that is not possible, try to use the Lua conditional code contained in the `<cond>` node in the header. You can either write simple conditional statements such as `player.credits() > 50e3`, where `return` gets automatically prepended, or you can write more complex code where you have to manually call `return` and return a boolean value.

Furthermore, when a mission or event passes the header and conditional Lua statements, the entire code gets compiled and run. This implies that all global variables are computed. If you load many graphics, shaders, or sound files as global values, this can cause a slowdown whenever the mission is started. An early issue with the visual novel framework was that all cargo missions were loading the visual novel framework that were loading lots of sounds and shaders. Since this was repeated for every mission in the mission computer, it created noticeable slowdowns. This was solved by relying on lazy loading and caching, and not just blindly loading graphics and audio files into global variables on library load.

### Making Aggressive Enemies

TODO Explain how to nudge the enemies without relying on pilot:control().

### Working with Player Fleets

TODO Explain how to detect and/or limit player fleets.
