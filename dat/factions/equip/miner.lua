local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local miner_outfits = eoutfits.merge{
   {
      -- Heavy Weapons
      -- Medium Weapons
      "Laser Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
      "Orion Beam",
      -- Small Weapons
      "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
      "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   },
   eoutfits.standard.set,
}

--[[
-- @brief Does Miner pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Choose parameters and make Pirateish
   local params = equipopt.params.choose( p )
   params.turret = 0.5
   params.launcher = 0.5
   params.bolt = 1.5
   params.type_range["Bolt Weapon"] = { min = 1 } -- need at least one weapon

   -- See cores
   local cores = ecores.get( p, { all="standard" } )

   -- Try to equip
   equipopt.optimize.optimize( p, cores, miner_outfits, params )
end
