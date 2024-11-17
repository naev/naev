--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pirate Shipcheck">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[
   Basically changes player's standing with pirates / marauders as necessary
   depending on their ship.
--]]
local pir = require 'common.pirate'
function create()
   hook.enter( "enter" )
end
function enter ()
   pir.updateStandings()
end
