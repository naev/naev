--[[

   Neutral Common Functions

--]]


-- Function for adding log entries for miscellaneous one-off missions.
function addMiscLog( text )
   shiplog.create( "misc", _("Miscellaneous"), _("Neutral") )
   shiplog.append( "misc", text )
end
