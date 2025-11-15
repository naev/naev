# Memory Model

By default, variables in Lua scripts are not saved when the player saves the game.
This means that all the values you have set up will be cleared if the player saves and loads.
This can lead to problems with scripts that do the following:

```lua
local dest

function create ()
   dest = spob.get("Caladan")

   -- ...

   hook.land( "land" )
end

function land ()
   if spob.cur() == dest then -- This is wrong!
      -- ...
   end
end
```

In the above script, a variable called `dest` is created, and when the mission is created, it gets set to `spob.get("Caladan")`.
Afterwards, it gets used in `land` which is triggered by a hook when the player lands.
For this mission, the value `dest` will be set as long as the player doesn't save and load.
When the player saves and loads, the value `dest` gets set to `nil` by default in the first line.
However, upon loading, the `create` function doesn't get run again, while the hook is still active.
This means that when the player lands, `spob.cur()` will be compared with `dest` will not have been set, and thus always be false.
In conclusion, the player will never be able to finish the mission!

How do we fix this?
The solution is the mission/event memory model.
In particular, all mission / event instances have a table that gets set called `mem`.
This table has the particular property of being *persistent*, i.e., even if the player saves and loads the game, the contents will not change!
We can then use this table and insert values to avoid issues with saving and loading games.
Let us update the previous code to work as expected with saving and loading.

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

We can see the changes are minimal.
We no longer declare the `dest` variable, and instead of setting and accessing `dest`, we use `mem.dest`, which is the `dest` field of the `mem` persistent memory table.
With these changes, the mission is now robust to saving and loading!

It is important to note that almost everything can be stored in the `mem` table, and this includes other tables.
However, make sure to not create loops, or it will hang the saving of the games.

The most common use of the persistent memory table `mem` is variables that keep track of the mission progress, such as if the player has delivered cargo or has talked to a certain NPC.
