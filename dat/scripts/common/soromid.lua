--[[

   Soromid Common Functions

--]]
local srm = {}

srm.prefix = "#b".._("SOROMID: ").."#0"

function srm.addComingOutLog( text )
   shiplog.create( "comingout", _("Coming Out"), _("Soromid") )
   shiplog.append( "comingout", text )
end

return srm
