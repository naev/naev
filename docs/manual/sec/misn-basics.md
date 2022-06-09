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

Each mission or event can have an infinite number of hooks enabled. Except for timer hooks, hooks do not get removed when run.

#### Timer Hooks

TODO

#### Pilot Hooks

TODO


### Translating
\label{sec:misn-basic-translation}

TODO

### Formatting Text
\label{sec:misn-basic-fmt}

TODO

### Visual Novel Framework
\label{sec:misn-basic-vn}

TODO

### Mission Computer Missions
\label{sec:misn-basic-computer}

TODO

### Ship Log

TODO
