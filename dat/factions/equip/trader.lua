local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'factions.equip.cores'
local eoutfits = require 'factions.equip.outfits'
local ecargo = require 'factions.equip.cargo'

-- Probability of cargo by class.
local cargo_chance = {
   ["Yacht"]         = 0.8,
   ["Luxury Yacht"]  = 0.8,
   ["Scout"]         = 0.1,
   ["Courier"]       = 0.8,
   ["Freighter"]     = 0.8,
   ["Armoured Transport"] = 0.8,
   ["Fighter"]       = 0.2,
   ["Bomber"]        = 0.2,
   ["Corvette"]      = 0.3,
   ["Destroyer"]     = 0.4,
   ["Cruiser"]       = 0.4,
   ["Carrier"]       = 0.6,
   ["Drone"]         = 0.1,
   ["Heavy Drone"]   = 0.1,
}

--[[
-- @brief Does Trader pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Choose parameters and make Pirateish
   local params = equipopt.params.choose( p )

   -- See cores
   local cores = ecores.get( p, { all="normal" } )

   -- Try to equip
   equipopt.equip( p, cores, eoutfits.standard.set, params )

   -- Add cargo
   if rnd.rnd() < cargo_chance[ p:ship():class() ] then
      ecargo.add( p )
   end
end
