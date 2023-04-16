--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Enlightenment">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Enlightenment</system>
</event>
--]]

local sys, sysr
local prevship

function create ()
   sys = system.get()
   sysr = system.radius()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Stop and play different music
   music.stop()

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Lesser", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )
   player.pilot():effectAdd("Astral Projection")

   hook.update( "update" )

   -- Anything will finish the event
   hook.enter( "done" )
end

-- Forces the player (and other ships) to stay in the radius of the system
function update ()
   for k,p in ipairs(pilot.get()) do
      local pos = p:pos()
      local d = pos:dist()
      if d > sysr then
         local _m, dir = pos:polar()
         p:setPos( vec2.newP( sysr, dir ) )
      end
   end
end

function done ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )

   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   evt.finish()
end
