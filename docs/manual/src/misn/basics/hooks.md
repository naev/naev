# Hooks

Hooks are the basic way missions and events can interact with the game.
They are accessed via the `hook.*` API and basically serve the purpose of binding script functions to specific in-game events or actions. A full list of the hook API is [available here](https://naev.org/api/modules/hook.html) and the API is always available in missions and events.
**Hooks are saved and loaded automatically.**

The basics to using hooks is as follows:

```lua
function create ()
   -- ...

   hook.land( "land" )
end

function land ()
   -- ...
end
```

In this example, at the end of the `create` function, the local function `land` is bound to the player landing with `hook.land`.
Thus, whenever the player lands, the script function `land` will be run.
All hook functions return a hook ID that can be used to remove the hook with `hook.rm`.
For example, we can write a slightly more complicated example as such:

```lua
function create ()
   -- ...

   mem.hook_land = hook.land( "land" )
   mem.hook_enter = hook.enter( "enter" )
end

function land ()
   -- ...
end

function enter ()
   hook.rm( mem.hook_land )
   hook.rm( mem.hook_enter )
end
```

The above example is setting up a `land` hook when the player lands, and an `enter` hook, which activates whenever the player enters a system by either taking off or jumping. Both hooks are stored in persistent memory, and are removed when the `enter` function is run when the player enters a system.

Each mission or event can have an infinite number of hooks enabled.
Except for `timer` and `safe` hooks, hooks do not get removed when run.

## Timer Hooks

Timer hooks are hooks that get run once when a certain amount of real in-game time has passed.
Once the hook is triggered, it gets removed automatically.
If you wish to repeat a function periodically, you have to create a new timer hook.
A commonly used example is shown below.

```lua
function create ()
   -- ...

   hook.enter( "enter" )
end

function enter ()
   -- ...

   hook.timer( 5, "dostuff" )
end

function dostuff ()
   if condition then
      -- ...
      return
   end
   -- ...
   hook.timer( 5, "dostuff" )
end
```

In this example, an `enter` hook is created and triggered when the player enters a system by taking off or jumping.
Then, in the `enter` function, a 5-second timer hook is started that runs the `dostuff` function when the time is up.
The `dostuff` function then checks a condition to do something and end, otherwise it repeats the 5-second hook.
This system can be used to, for example, detect when the player is near a pilot or position, or display periodic messages.

Timer hooks persist even when the player lands and takes off.
If you wish to clear them, please use `hook.timerClear()`, which will remove all the timers created by the mission or event calling the function.
This can be useful in combination with `hook.enter`.

## Pilot Hooks

When it comes to pilots, hooks can also be used.
However, given that pilots are not saved, the hooks are not saved either.
The hooks can be made to be specific to a particular pilot, or apply to any pilot.
In either case, the pilot triggering the hook is passed as a parameter.
An illustrative example is shown below:

```lua
function enter ()
   -- ...

   local p = pilot.add( "Llama", "Independent" )
   hook.pilot( p, "death", "pilot_died" )
end

function pilot_died( p )
   -- ...
end
```

In the above example, when the player enters a system with the `enter` function, a new pilot `p` is created, and a `"death"` hook is set on that pilot.
Thus, when the pilot `p` dies, the `pilot_dead` function will get called.
Furthermore, the `pilot_died` function takes the pilot that died as a parameter.

There are other hooks for a diversity of pilot actions that are documented in [the official API documentation](https://naev.org/api/modules/hook.html#pilot), allowing for full control of pilot actions.
