--[[

   Shadow Common Functions

--]]
local shadow = {}

function shadow.addLog( text )
   shiplog.create( "shadow", _("Shadow"), _("Shadow") )
   shiplog.append( "shadow", text )
end

shadow.rewards = {
   shadowrun = 400e3,
   shadowvigil = 700e3,
   darkshadow = 1e6,
}

return shadow
