--[[

   Shark Common Functions

--]]
local shark = {}

function shark.pirateFaction ()
   local enemy = faction.dynAdd( "Pirate", "Wanted Pirate", _("Wanted Pirate"), {clear_enemies=true, clear_allies=true} )
   return enemy
end

function shark.addLog( text )
   shiplog.create( "shark", _("Nexus Shipyards"), _("Nexus Shipyards") )
   shiplog.append( "shark", text )
end

return shark
