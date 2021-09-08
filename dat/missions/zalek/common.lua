--[[

   Za'lek Common Functions

--]]

function zlk_addSciWrongLog( text )
   shiplog.create( "zlk_sciwrong", _("Science Gone Wrong"), _("Za'lek") )
   shiplog.append( "zlk_sciwrong", text )
end

function zlk_addNebuResearchLog( text )
   shiplog.create( "zlk_neburesearch", _("Nebula Research"), _("Za'lek") )
   shiplog.append( "zlk_neburesearch", text )
end

-- Function for adding log entries for miscellaneous one-off missions.
function zlk_addMiscLog( text )
   shiplog.create( "zlk_misc", _("Miscellaneous"), _("Za'lek") )
   shiplog.append( "zlk_misc", text )
end

-- Checks to see if the player has a Za'lek ship.
function zlk_hasZalekShip()
   local shipname = player.pilot():ship():nameRaw()
   return string.find( shipname, "Za'lek" ) ~= nil
end
