--[[
<?xml version='1.0' encoding='utf8'?>
<event name="System Tour">
 <trigger>none</trigger>
 <chance>0</chance>
 <flags>
 </flags>
</event>
--]]
--[[
-- Tests system / overlay rendering by screenshotting each one.
-- To use: land, put this line in the Lua console, and take off:
-- naev.eventStart("System Tour")
--]]
local systems, idx
-- luacheck: globals screenshot teleport (Hook functions passed by name)

function create()
   player.pilot():setInvincible()
   systems = system.getAll()
   idx = 1
   hook.takeoff("teleport")
end

function teleport()
   player.teleport( systems[idx], true )
   pilot.clear()
   hook.safe("screenshot")
end

function screenshot()
   player.screenshot()
   idx = idx + 1
   if systems[idx] ~= nil then
      hook.safe("teleport")
   end
end
