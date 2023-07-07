--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test Conditionals">
 <location>load</location>
 <cond>
   return naev.conf().devmode
 </cond>
 <chance>100</chance>
</event>
--]]
--[[
   Tests mission/event conditionals.
--]]
function create()
   -- Run over all missions and test conditionals
   for k,m in ipairs(naev.missionList()) do
      naev.missionTest( m )
   end
end
