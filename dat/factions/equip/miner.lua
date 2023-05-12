local equipopt = require 'equipopt'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local miner_outfits = eoutfits.merge{
   {
      -- Heavy Weapons
      -- Medium Weapons
      "Mining Lance MK2",
      "Laser Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
      "Orion Beam",
      -- Small Weapons
      "Mining Lance MK1",
      "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
      "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
      -- Other mining stuff
      "S&K Heavy Plasma Drill",
      "S&K Plasma Drill",
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
   params.beam = 1.5
   params.prefer["Mining Lance MK1"] = 2
   params.prefer["Mining Lance MK2"] = 2
   if rnd.rnd() < 0.2 then
      params.prefer["S&K Heavy Plasma Drill"] = 2
      params.prefer["S&K Plasma Drill"] = 2
   end

   -- See cores
   local cores = ecores.get( p, { all="standard" } )

   -- Try to equip
   return equipopt.optimize.optimize( p, cores, miner_outfits, params )
end
