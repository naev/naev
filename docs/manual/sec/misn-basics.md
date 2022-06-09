## Basics

In this section we will discuss basic and fundamental aspects of mission and event developments that you will have to take into account in almost all cases.

### Memory Model
\label{sec:misn-basic-mem}

By default, variables in Lua scripts are not saved when the player saves the game. This means that all the values you have set up will be cleared if the player saves and loads. This can lead to problems wit scripts that do the following:

```lua
local dest

function create ()
   dest = spob.get("Caladan")

   -- ...

   hook.land( "land" )
end

function land ()
   if spob.cur() == dest then
      -- ...
   end
end
```

In the above script, a variable called `dest` is created, and when the mission is created, it gets set to `spob.get("Caladan")`. Afterwards, it gets used in `land` which is triggered by a hook when the player lands. For this mission, the value `dest` will be set as long as the player doesn't save and load. When the player saves and loads, the value `dest` gets set to `nil` by default in the first line. However, upon loading, the `create` function doesn't get run again, while the hook is still active. This means that when the player lands, `spob.cur()` will be compared with `dest` will not have been set, and thus always be false. In conclusion, the player will never be able to finish the mission!

How do we fix this? The solution is the mission/event memory model. In particular, all mission / event instances have a table that gets set called `mem`. This table has the particular property of being *persistent*, i.e., even if the player saves and loads the game, the contents will not change! We can then use this table and insert values to avoid issues with saving and loading games. Let us update the previous code to work as expected with saving and loading.

```lua
function create ()
   mem.dest = spob.get("Caladan")

   -- ...

   hook.land( "land" )
end

function land ()
   if spob.cur() == mem.dest then
      -- ...
   end
end
```

We can see the changes are minimal. We no longer declare the `dest` variable, and instead of setting and accessing `dest`, we use `mem.dest`, which is the `dest` field of the `mem` persistent memory table. With these changes, the mission is now robust to saving and loading!

It is important to note that almost everything can be stored in the `mem` table, and this includes other tables. However, make sure to not create loops or it will hang the saving of the games.

The most common use of the persistent memory table `mem` is variables that keep track of the mission progress, such as if the player has delivered cargo or has talked to a certain NPC.

### Hooks
\label{sec:misn-basic-hooks}

Hooks are the basic way missions and events can interact with the game. They are accessed via the `hook.*` API and basically serve the purpose of binding script functions to specific in-game events or actions. A full list of the hook API is [available here](https://naev.org/api/modules/hook.html). **Hooks are saved and loaded automatically.**

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

In this example, at the end of the `create` function, the local function `land` is bound to the player landing with `hook.land`. Thus, whenever the player lands, the script function `land` will be run. All hook functions return a hook ID that can be used to remove the hook with `hook.rm`. For example, we can write a slightly more complicated example as such:

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

Each mission or event can have an infinite number of hooks enabled. Except for `timer` and `safe` hooks, hooks do not get removed when run.

#### Timer Hooks

Timer hooks are hooks that get run once when a certain amount of real in-game time has passed. Once the hook is triggered, it gets removed automatically. If you wish to repeat a function periodically, you have to create a new timer hook. A comomnly used example is shown below.

```
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

In this example, an `enter` hook is created and triggered when the player enters a system by taking off or jumping. Then, in the `enter` function, a 5 second timer hook is started that runs the `dostuff` function when the time is up. The `dostuff` function then checks a condition to do something and end, otherwise it repeats the 5 second hook. This system can be used to, for example, detect when the player is near a pilot or position, or display periodic messages.

#### Pilot Hooks

When it comes to pilots, hooks can also be used. However, given that pilots are not saved, the hooks are not saved either. The hooks can be made to be specific to a particular pilot, or apply to any pilot. In either case, the pilot triggering the hook is passed as a parameter. An illustrative example is shown below:

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

In the above example, when the player enters a system with the `enter` function, a new pilot `p` is created, and a `"death"` hook is set on that pilot. Thus, when the pilot `p` dies, the `pilot_dead` function will get called. Furthermore, the `pilot_died` function takes the pilot that died as a parameter.

There are other hooks for a diversity of pilot actions that are documented in [the official API documentation](https://naev.org/api/modules/hook.html#pilot), allowing for full control of pilot actions.

### Translating
\label{sec:misn-basic-translation}

Naev supports translation through [Weblate](https://hosted.weblate.org/projects/naev/naev/). However, in order for translations to be used you have to mark strings as translatable. This is done with a [gettext](https://www.gnu.org/software/gettext/) compatible interface. In particular, the following functions are provided:

* `_()`: This function takes a string, marks it as translatable, and returns the translated version.
* `N_()`: This function takes a string, marks it as translatable, however, it returns the *untranslated* version of the string.
* `n_()`: Takes two strings related to a number quantity and return the translated version that matches the number quantity. This is because some languages translate number quantities differently. For example "1 apple", but "2 apple**s**".

In general, you want to use `_()` and `n_()` to envelope all strings that are being shown to the player, which will allow for translations to work without extra effort. For example, when defining a new mission you want to translate all the strings as shown below:

```lua
misn.setTitle( _("My Mission") )
misn.setDesc( _("You have been asked to do lots of fancy stuff for a very fancy individual. How fancy!") )
misn.setReward( _("Lots of good stuff!") )
```

Note that `_()` and friends all assume that you are inputting strings in English.

It is important to note that strings not shown to the player, e.g., strings representing faction names or ship names, do not need to be translated! So when adding a pilot you can just use directly the correct strings:

```lua
pilot.add( "Hyena", "Mercenary" )
```

### Formatting Text
\label{sec:misn-basic-fmt}

TODO

### System Claiming
\label{sec:misn-basic-claims}

TODO

### Visual Novel Framework
\label{sec:misn-basic-vn}

TODO

### Mission Computer Missions
\label{sec:misn-basic-computer}

TODO

### Ship Log

TODO
