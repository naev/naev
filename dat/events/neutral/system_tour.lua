--[[
<?xml version='1.0' encoding='utf8'?>
<event name="System Tour">
  <trigger>load</trigger>
  <chance>100</chance>
  <cond>false</cond>
  <flags>
  </flags>
 </event>
 --]]
--[[
-- Tests system / overlay rendering by screenshotting each one.
-- To use: land, put this line in the Lua console, and take off:
-- naev.eventStart("System Tour")
--]]

function create()
   player.pilot():setInvincible()
   systems = system.getAll()
   idx = 1
   hook.takeoff('teleport')
end

function teleport()
   player.teleport(systems[idx])
   pilot.clear()
   hook.safe('screenshot')
end

function screenshot()
   player.screenshot()
   idx = idx + 1
   if systems[idx] ~= nil then
      hook.safe('teleport')
   end
end
