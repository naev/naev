local equipopt = require 'equipopt'
--[[
-- @brief Does Marauder pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local ps = p:ship()
   local _nw, nu, ns = ps:slots()

   -- Use fewer slots for now (would be better to use budget)
   local params = {
      max_util = rnd.rnd( math.floor(nu*0.5), nu ),
      max_stru = rnd.rnd( math.floor(ns*0.5), ns ),
   }

   return equipopt.pirate( p, params )
end
