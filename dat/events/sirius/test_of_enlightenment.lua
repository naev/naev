--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Enlightenment">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Enlightenment</system>
</event>
--]]

local sys

function create ()
   sys = system.get()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   sys:setHidden(false)

   -- Stop and play different music
   music.stop()

   -- Anything will finish the event
   hook.enter( "done" )
end

function done ()
   sys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
   evt.finish()
end
