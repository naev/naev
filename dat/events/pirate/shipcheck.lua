--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pirate Shipcheck">
 <location>enter</location>
 <chance>100</chance>
</event>
--]]
--[[
   Basically changes player's standing with pirates / marauders as necessary
   depending on their ship.
--]]
local pir = require 'common.pirate'
function create()
   pir.updateStandings()
   evt.finish()
end
