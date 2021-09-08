--[[

   Shark Common Functions

--]]
local shark = {}

function shark.addLog( text )
   shiplog.create( "shark", _("Nexus Shipyards"), _("Nexus Shipyards") )
   shiplog.append( "shark", text )
end

return shark
