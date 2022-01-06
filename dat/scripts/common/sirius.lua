--[[

   Sirius Common Functions

--]]
local srs = {}

srs.prefix = "#b".._("SRS: ").."#0"

function srs.addAcHackLog( text )
   shiplog.create( "achack", _("Academy Hack"), _("Sirius") )
   shiplog.append( "achack", text )
end


function srs.addHereticLog( text )
   shiplog.create( "heretic", _("Heretic"), _("Sirius") )
   shiplog.append( "heretic", text )
end

return srs
