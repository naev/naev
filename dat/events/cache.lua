--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Cache Cleaner">
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Cleans some stuff in the cache on load to avoid certain minor issues
--]]
function create ()
   local c = naev.cache()
   c.misn_patrols = nil -- Patrol missions
   c.misn_escorts = nil -- Trader escort missions
   evt.finish()
end
