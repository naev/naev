--[[

   Sirius Common Functions

--]]


function srs_addAcHackLog( text )
   shiplog.create( "achack", _("Academy Hack"), _("Sirius") )
   shiplog.append( "achack", text )
end


function srs_addHereticLog( text )
   shiplog.create( "heretic", _("Heretic"), _("Sirius") )
   shiplog.append( "heretic", text )
end
