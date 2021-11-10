local equipopt = require 'equipopt'
local ecargo = require 'equipopt.cargo'

-- Probability of cargo by class.
local cargo_chance = {
   ["Yacht"]         = 0.95,
   ["Courier"]       = 0.95,
   ["Freighter"]     = 0.95,
   ["Armoured Transport"] = 0.95,
   ["Bulk Freighter"]= 0.95,
}

--[[
-- @brief Does Dvaered pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local ret = equipopt.empire( p )
   -- Add cargo
   local cc = cargo_chance[ p:ship():class() ]
   if cc and rnd.rnd() < cc then
      ecargo.add( p )
   end
   return ret
end

