## Special Bounties for Naev

These are special bounties that will appear in the mission computer.
The player can only have one active at a time, and they allow for more tailoured and difficult challenges with potentially unique loot.

### Adding

You can add a new special bounty by creating a lua file in `dat/events/special_bounty/bounties/` (in the case of a plugin, you do not need the first `dat/`).
The file should return a table with specific fields that determine what the bounty is.
These fields are documented below:

* `var` (optional): should be a unique name of a player variable to mark as true when the mission is completed. If not set, it defaults to the name of the lua file.
* `title`: title of the bounty. This will appear as the mission title in both the mission computer and on screen display.
* `desc`: description of the mounty that appears in the mission computer and info window.
* `escorts` (optional): adds a short description of what escorts the pilot will have. If not set doesn't say anytthing.
* `reward`: what to give the player. If it is a number, it is the number of credits to pay.
* `system`: system where the bounty should be.
* `name`: name of the pilot
* `payingfaction`: faction that will give reputation and appear in the title in the mission computer.
* `reputation`: amount of reputation to give.
* `targetfaction`: faction that will be used for the target. This will determine the equipment
* `staticfaction` (optional): if true will use the real faction instead of creating a new dynamic one that is not hostile nor allied with anyone else.
* `alive_only`: whether or not the player has to capture them alive.
* `ships`: the ships to spawn in the fleet. The first will be the flagship and the target. However, this can be overriden with `spawnfunc`, in which case the first ship will be used to create the description only.
* `spawnfunc` (optional): a Lua function that will be run when spawning the target. This takes two parameters: the first is the bounty data table, and the second is the location parameter that should be used with `pilot.add`. An example is shown below.

### Spawn Function

Below is an example of a documented spawn script.

```lua
-- These includes should be added at the top of the file.
local bhelp = require "events.special_bounty.helpers"
local bounty = require "common.bounty"

-- This function should be set as the spawnfunc field of the table returned
-- It should return the target pilot
local function spawnfunc( b, params )
   -- Get the faction of the bounty, this will automatically be a dynamic or normal faction depending on the settings
   local fct = bounty.get_faction()

   -- Add the target pilot, this pilot can be modified as wanted
   local p = pilot.add( b.targetship[1], fct, params, b.targetname )

   -- Add 200 points worth of mercenary shps, however, they should not be larger than the target pilot's ship
   -- These are set to follow the target who is the leader
   for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 200 )) do
      local e = pilot.add( s, fct, params )
      e:setLeader(p)
   end

   -- Return the target pilot
   return p
end
```
