## Tips and Tricks

This section contains some tricks and tips for better understanding how to do specific things in missions and events.

### Optimizing Loading

It is important to understand how missions and events are loaded. The headers are parsed at the beginning of the game and stored in memory. Whenever a trigger (entering a system, landing on a spob, etc.) happens, the game runs through all the missions and events to check to see if they should be started. The execution is done in the following way:

1. Header statements are checked (*e.g.*, unique missions that are already done are discarded)
1. Lua conditional code is compiled and run
1. Lua code is compiled and run
1. `create` function is run

In general, since many events and missions can be checked at every frame, it is important to try to cull them as soon as possible. When you can, use location or faction filters to avoid having missions and events appear in triggers you don't wish them to. In the case that is not possible, try to use the Lua conditional code contained in the `<cond>` node in the header. You can either write simple conditional statements such as `player.credits() > 50e3`, where `return` gets automatically prepended, or you can write more complex code where you have to manually call `return` and return a boolean value. Do note, however, that it is not possible to reuse variables or code in the `<cond>` node in the rest of the program. If you have to do expensive computations and wish to use the variables later on, it is best to put the conditional code in the `create` function and abort the mission or event with `misn.finish(false)` or `evt.finish(false)`, respectively.

Furthermore, when a mission or event passes the header and conditional Lua statements, the entire code gets compiled and run. This implies that all global variables are computed. If you load many graphics, shaders, or sound files as global values, this can cause a slowdown whenever the mission is started. An early issue with the visual novel framework was that all cargo missions were loading the visual novel framework that were loading lots of sounds and shaders. Since this was repeated for every mission in the mission computer, it created noticeable slowdowns. This was solved by relying on lazy loading and caching, and not just blindly loading graphics and audio files into global variables on library load.

### Global Cache

In some cases that you want to load large amount of data once and reuse it throughout different instances of events or missions, it is possible to use the global cache with `naev.cache()`. This function returns a table that is accessible by all the Lua code. However, this cache is cleared every time the game starts. You can not rely on elements in this cache to be persistent. It is common to wrap around the cache with the following code:

```lua
local function get_calculation ()
   local nc = naev.cache()
   if nc.my_calculation then
      return nc.my_calculation
   end
   nc.my_calculation = do_expensive_calculation ()
   return nc.my_calculation
end
```

The above code tries to access data in the cache. However, if it does not exist (by default all fields in Lua are nil), it will do the expensive calculation and store it in the cache. Thus, the first call of `get_calculation()` will be slow, however, all subsequent calls will be very fast as no `do_expensive_calculation()` gets called.

### Finding Natural Pilots \naev

In some cases, you want a mission or event to do things with naturally spawning pilots, and not spawn new ones. Naturally spawned pilots have the `natural` flag set in their memory. You can access this with `p:memory().natural` and use this to limit boarding hooks and the likes to only naturally spawned pilots. An example would be:

```lua
function create ()
   -- ...
   hook.board( "my_board" )
end

function my_board( pilot_boarded )
   if not pilot_boarded:memory().natural then
      return
   end
   -- Do something with natural pilots here
end
```

In the above example, we can use a board hook to control when the player boards a ship, and only handle the case that naturally spawning pilots are boarded.

### Making Aggressive Enemies

TODO Explain how to nudge the enemies without relying on pilot:control().

### Working with Player Fleets

TODO Explain how to detect and/or limit player fleets.
