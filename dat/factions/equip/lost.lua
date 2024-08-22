local equipopt = require 'equipopt'
--[[
-- @brief Does Lost pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local params = {
      disable = 0, -- THEY WANT BLOOD
   }
   return equipopt.pirate( p, params )
end
