--[[

   Neutral Common Functions

--]]
local neu = {}

-- Function for adding log entries for miscellaneous one-off missions.
function neu.addMiscLog( text )
   shiplog.create( "misc", _("Miscellaneous"), _("Neutral") )
   shiplog.append( "misc", text )
end

return neu
