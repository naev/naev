local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local ecargo = require 'equipopt.cargo'

-- Probability of cargo by class.
local cargo_chance = {
   ["Yacht"]         = 0.8,
   ["Courier"]       = 0.8,
   ["Freighter"]     = 0.8,
   ["Armoured Transport"] = 0.8,
   ["Bulk Freighter"]= 0.9,
   ["Scout"]         = 0.1,
   ["Interceptor"]   = 0.2,
   ["Fighter"]       = 0.2,
   ["Bomber"]        = 0.2,
   ["Corvette"]      = 0.3,
   ["Destroyer"]     = 0.4,
   ["Cruiser"]       = 0.4,
   ["Battleship"]    = 0.4,
   ["Carrier"]       = 0.6,
}

--[[
-- @brief Does Trader pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Choose parameters and make Pirateish
   local params = equipopt.params.choose( p )
   params.rnd = params.rnd * 1.5
   params.max_mass = 0.8 + 0.2*rnd.rnd() -- want space for cargo!

   -- See cores
   local cores = ecores.get( p, { all="standard" } )

   -- Try to equip
   equipopt.optimize.optimize( p, cores, eoutfits.standard.set, params )

   -- Add cargo
   local cc = cargo_chance[ p:ship():class() ]
   if cc and rnd.rnd() < cc then
      ecargo.add( p )
   end
end
