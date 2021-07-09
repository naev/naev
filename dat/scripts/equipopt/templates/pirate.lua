local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local pirate_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Lancelot Fighter Bay",
   "Hyena Fighter Dock",
   "Hyena Fighter Bay",
   "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun",
   "Plasma Turret MK2", "Orion Beam", "EMP Grenade Launcher",
   "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
   "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
   "Orion Lance", "Ion Cannon",
   -- Small Weapons
   "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
   "Laser Turret MK1", "Razor Turret MK1", "Turreted Gauss Gun",
   "Plasma Turret MK1", "Particle Beam",
   "Unicorp Mace Launcher", "Unicorp Banshee Launcher",
   -- Utility
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   "Scanning Combat AI",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster",
}}

local pirate_class = { "standard", "elite" }

local pirate_params = {
   ["Pirate Kestrel"] = function () return {
         type_range = {
            ["Launcher"] = { max = 2 },
         },
      } end,
}
local pirate_cores = {
   ["Hyena"] = function (p) return ecores.get( p, { all=pirate_class, heavy=false } ) end,
   ["Pirate Shark"] = function (p) return ecores.get( p, { all=pirate_class, heavy=false } ) end,
   ["Pirate Kestrel"] = function (p)
         local heavy = rnd.rnd() < 0.3
         if heavy then
            return ecores.get( p, { all=pirate_class, heavy=heavy } )
         end
         local c = ecores.get( p, { systems=pirate_class, hulls=pirate_class, heavy=false } )
         table.insert( c, choose_one{ "Nexus Bolt 4500 Engine", "Krain Remige Engine", "Tricon Typhoon Engine", } )
         return c
      end,
}

--[[
-- @brief Does Pirate pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_pirate( p, opt_params )
   opt_params = opt_params or {}
   local ps = p:ship()
   local pc = ps:class()
   local sname = ps:nameRaw()

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p )
   -- Prefer to use the Pirate utilities
   params.prefer["Scanning Combat AI"]      = 100
   params.weap = 2 -- Focus on weapons!
   -- much more diversity for pirates
   params.max_same_stru = 1
   params.max_same_util = 1
   params.rnd = params.rnd * 1.5
   if pc == "Fighter" or pc == "Bomber" then
      params.max_same_weap = 1
   else
      params.max_same_weap = 2
   end
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = pirate_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   local cores
   local pircor = pirate_cores[ sname ]
   if pircor then
      cores = pircor( p )
   else
      cores = ecores.get( p, { all=pirate_class } )
   end

   -- Try to equip
   return optimize.optimize( p, cores, pirate_outfits, params )
end

return equip_pirate
