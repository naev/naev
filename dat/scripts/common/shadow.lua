--[[

   Shadow Common Functions

--]]
local shadow = {}

function shadow.addLog( text )
   shiplog.create( "shadow", _("Shadow"), _("Shadow") )
   shiplog.append( "shadow", text )
end

return shadow
