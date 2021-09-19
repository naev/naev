--[[

   Soromid Common Functions

--]]
local srm = {}

function srm.addComingOutLog( text )
   shiplog.create( "comingout", _("Coming Out"), _("Soromid") )
   shiplog.append( "comingout", text )
end

return srm
