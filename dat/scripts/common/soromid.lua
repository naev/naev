--[[

   Soromid Common Functions

--]]


function srm_addComingOutLog( text )
   shiplog.create( "comingout", _("Coming Out"), _("Soromid") )
   shiplog.append( "comingout", text )
end
