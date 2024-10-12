--[[

   Soromid Common Functions

--]]
local srm = {}

-- Should be brownish-red, but we don't have that option.
srm.prefix = "#b".._("SOROMID: ").."#0"

local function isbioship( s )
   return s:tags().bioship
end
function srm.playerHasBioship()
   if isbioship( player.pilot():ship() ) then
      return true
   end
   for k,s in ipairs(player.ships()) do
      if isbioship( s.ship ) then
         return true
      end
   end
   return false
end

return srm
