# Getting Started

Missions and events share the same overall structure in which there is a large Lua comment at the top containing all sorts of meta-data, such as where it appears, requirements, etc.
Once the mission or event is started, the obligatory `create` function entry point is run.

## Mission Header

Let us start by writing a simple [mission header](./basics/header.md).
This will be enclosed by long Lua comments `--[[` and `--]]` in the file.
Below is our simple header.

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="My First Mission">
 <unique />
 <chance>100</chance>
 <location>Bar</location>
</mission>
--]]
```

The mission is named "My First Mission" and has a 50% chance of appearing in any spaceport bar.
Furthermore, it is marked unique so that once it is successfully completed, it will not appear again to the same player.

## Entry Point `create`

Now, we can start coding the actual mission.
This all begins with the `create ()` function.
Let us write a simple one to create an NPC at the Spaceport Bar where the mission appears:

```lua
function create ()
   misn.setNPC( _("A human."),
         "neutral/unique/youngbusinessman.avif",
         _("A human wearing clothes.") )
end
```

The create function in this case is really simple, it only creates a single NPC with `misn.setNPC`.
Please note that only a single NPC is supported with `misn.setNPC`, if you want to use more NPC you would have to use `misn.npcAdd` which is much more flexible and not limited to mission givers.
There are two important things to note:

1. All human-readable text is enclosed in `_()` for translations.
  In principle, you should always use `_()` to enclose any text meant for the user to read, which will allow the [translation system](./basics/translation.md) to automatically deal with it.
1. There is an image defined as a string.
  In this case, this refers to an image in `gfx/portraits/`.
  Note that Naev uses a virtual filesystem and the exact place of the file may vary depending on where it is set up.

With that set up, the mission will now spawn an NPC with 50% chance at any Spaceport Bar, but they will not do anything when approached.
This is because we have not defined an `accept()` function.
This function is only necessary when either using `misn.npcAdd` or creating mission computer missions.
So let us define that which will determine what happens when the NPC is approached as follows:

```lua
local vntk = require "vntk"
local fmt = require "format"

local reward = 50e3 -- This is equivalent to 50000, and easier to read

function accept ()
   -- Make sure the player has space
   if player.pilot():cargoFree() < 1 then
      vntk.msg( _("Not Enough Space"),
            _("You need more free space for this mission!") )
      return
   end

   -- We get a target destination
   mem.dest, mem.destsys = spob.getS( "Caladan" )

   -- Ask the player if they want to do the mission
   if not vntk.yesno( _("Apples?"),
         fmt.f(_("Deliver apples to {spb} ({sys})?"),
               {spb=mem.dest,sys=mem.destsys}) ) then
      -- Player did not accept, so we finish here
      vntk.msg(_("Rejected"),_("Your loss."))
      misn.finish(false) -- Say the mission failed to complete
      return
   end

   misn.accept() -- Have to accept the mission for it to be active

   -- Set mission details
   misn.setTitle( _("Deliver Apples") )
   misn.setReward( fmt.credits( reward ) )
   local desc = fmt.f(_("Take Apples to {spb} ({sys})."),
         {spb=mem.dest,sys=mem.destsys})
   misn.setDesc( desc )

   -- On-screen display
   misn.osdCreate( _("Apples"), { desc } )

   misn.cargoAdd( "Food", 1 ) -- Add cargo
   misn.markerAdd( mem.dest ) -- Show marker on the destination

   -- Hook will trigger when we land
   hook.land( "land" )
end
```

This time it's a bit more complicated than before.
Let us try to break it down a bit.
The first line includes the `vntk` library, which is a small wrapper around the `vn` Visual Novel library.
This allows us to show simple dialogues and ask the player questions.
We also include the `format` library to let us format arbitrary text, and we also define the local reward to be 50,000 credits in exponential notation.

The function contains of 3 main parts:

1. We first check to see if the player has enough space for the apples with `player.pilot():cargoFree()` and display a message and return from the function if not.
1. We then ask the player if then ask the player if they want to deliver apples to **Caladan** and if they don't, we give a message and return from the function.
1. Finally, we accept the mission, adding it to the player's active mission list, set the details, add the cargo to the player, and define a hook on when the player lands to run the final part of the mission.
  Functions like `misn.markerAdd` add markers on the spob the player has to go to, making it easier to complete the mission.
  The On-Screen Display (OSD) is also set with the mission details to guide the player with `misn.osdCreate`.

Some important notes.

* We use `fmt.f` to format the strings. In this case, the `{spb}` will be replaced by the `spb` field in the table, which corresponds to the name of the `mem.dest` spob. This is further explained in [Formatting Text](./basics/format.md).
* Variables don't get saved unless they are in the `mem` table. This table gets populated again every time the save game gets loaded. More details in [Memory Model](./basics/memory.md)
* You have to pass function names as strings to the family of `hook.*` functions. More details on hooks in [Hooks](./baiscs/hooks.md).

Now this gives us almost the entirety of the mission, but a last crucial component is missing: we need to reward the player when they deliver the cargo to **Caladan**.
We do this by exploiting the `hook.land` that makes it so our defined `land` function gets called whenever the player lands.
We can define one as follows:

```lua
local neu = require "common.neutral"
function land ()
   if spob.cur() ~= mem.dest then
      return
   end

   vn.msg(_("Winner"), _("You win!"))
   neu.addMiscLog( _("You helped deliver apples!") )
   player.pay( reward )
   misn.finish(true)
end
```

We can see it's very simple.
It first does a check to make sure the landed planet `spob.cur()` is indeed the destination planet `mem.dest`.
If not, it returns, but if it is, it'll display a message, add a message to the ship log, pay the player, and finally finish the mission with `misn.finish(true)`.
Remember that since this is defined to be a unique mission, once the mission is done it will not appear again to the same player.

That concludes our very simple introduction to mission writing.
Note that it doesn't handle things like playing victory sounds, nor other more advanced functionality.
However, please refer to the [full example](./src/misn/example.md) that covers more advanced functionality.
