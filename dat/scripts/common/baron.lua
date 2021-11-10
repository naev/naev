--[[

   Baron Common Functions

--]]
local baron = {}

-- Function for adding log entries for miscellaneous one-off missions.
function baron.addLog( text )
   shiplog.create( "baron", _("Baron"), _("Baron") )
   shiplog.append( "baron", text )
end

baron.rewards = {
   baron = 300e3,
   prince = 500e3, -- The price of each artifact will always be 15% of this, so at most the player will be paid 85% and at least 55%.
}

return baron
