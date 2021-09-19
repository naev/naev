--[[

   Baron Common Functions

--]]
local baron = {}

-- Function for adding log entries for miscellaneous one-off missions.
function baron.addLog( text )
   shiplog.create( "baron", _("Baron"), _("Baron") )
   shiplog.append( "baron", text )
end

return baron
