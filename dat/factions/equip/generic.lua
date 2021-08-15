local equipopt = require 'equipopt'
local ecargo = require 'equipopt.cargo'

-- Probability of cargo by class.
local cargo_chance = {
   ["Yacht"]         = 0.8,
   ["Courier"]       = 0.8,
   ["Freighter"]     = 0.8,
   ["Armoured Transport"] = 0.8,
   ["Bulk Freighter"]= 0.9,
   ["Scout"]         = 0.1,
   ["Interceptor"]   = 0.1,
   ["Fighter"]       = 0.2,
   ["Bomber"]        = 0.2,
   ["Corvette"]      = 0.3,
   ["Destroyer"]     = 0.4,
   ["Cruiser"]       = 0.4,
   ["Battleship"]    = 0.4,
   ["Carrier"]       = 0.6,
}

--[[
-- @brief Does Generic pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_generic( p )
   local params = {
      rnd = 0.3,
      max_mass = 0.9 + 0.2*rnd.rnd(),
   }

   -- Do equipment
   local ret = equipopt.generic( p, params )

   -- Add cargo
   local cc = cargo_chance[ p:ship():class() ]
   if cc and rnd.rnd() < cc then
      ecargo.add( p )
   end

   return ret
end
