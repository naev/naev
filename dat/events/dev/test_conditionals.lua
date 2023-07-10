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
      if not inlist( {
         "computer", "bar", "land",
      }, m.loc ) then
         naev.missionTest( m.name )
      end
   end
   hook.load("load_game")
end

function load_game ()
   for k,m in ipairs(naev.missionList()) do
      if inlist( {
         "computer", "bar", "land",
      }, m.loc ) then
         naev.missionTest( m.name )
      end
   end
   evt.finish()
end
