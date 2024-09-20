local equipopt = require 'equipopt'

local intrinsic_taint = outfit.get("Wild Space Taint")

--[[
-- @brief Does Lost pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local params = {
      disable = 0, -- THEY WANT BLOOD
   }
   p:outfitAddIntrinsic( intrinsic_taint ) -- So the player can capture it
   return equipopt.pirate( p, params )
end
