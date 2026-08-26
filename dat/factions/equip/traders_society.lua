
local equipopt = require 'equipopt'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local ecargo = require 'equipopt.cargo'

-- Probability of cargo by class.
local cargo_chance = {
   ["Yacht"]         = 0.95,
   ["Courier"]       = 0.95,
   ["Freighter"]     = 0.95,
   ["Armoured Transport"] = 0.95,
   ["Bulk Freighter"]= 0.95,
}

local trader_outfits = eoutfits.merge{
   {
      "Gorgon Lancelot Bay",
      "Cargo Damper",
   },
   eoutfits.standard.set,
}

--[[
-- @brief Does Trader pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Choose parameters and make Traderish
   local params   = equipopt.params.choose( p )
   params.rnd     = params.rnd * 1.5
   params.damage  = 0.9
   params.disable = 1.1

   -- See cores
   local core_type = ((rnd.rnd() > 0.5) and "elite") or "standard"
   local cores = ecores.get( p, { all=core_type } )

   -- Try to equip
   local ret = equipopt.optimize.optimize( p, cores, trader_outfits, params )

   -- Add cargo
   local cc = cargo_chance[ p:ship():class() ]
   if cc and rnd.rnd() < cc then
      ecargo.add( p )
   end

   return ret
end
