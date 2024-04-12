--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Sol - The Final Frontier">
 <location>enter</location>
 <chance>100</chance>
 <system>Sol</system>
</event>
--]]
--[[
   My name is Sol, you killed my father. Prepare to die.
--]]
function create ()
   hook.update("update")
   hook.enter("enter")
end
local elapsed = 0
function update( dt )
   elapsed = elapsed + dt
   for k,p in ipairs(pilot.get()) do
      p:damage( math.pow(elapsed,1.3), 0., 1., "nebula" )
   end
end
function enter ()
   evt.finish()
end
