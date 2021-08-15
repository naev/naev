local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local soromid_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Soromid Brigand Fighter Bay",
   "Heavy Laser Turret", "Grave Beam",
   "BioPlasma Talon Stage X",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
   "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Plasma Cluster Cannon",
   "BioPlasma Fang Stage X",
   -- Small Weapons
   "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
   "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   "BioPlasma Claw Stage X",
   "BioPlasma Stinger Stage X",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Bio-Neural Combat AI", "Nexus Stealth Coating",
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

local soromid_params = {
   ["Soromid Arx"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local soromid_cores = {
}

--[[
-- @brief Does Soromid pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_soromid( p, opt_params  )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Soromidish
   local params = eparams.choose( p )
   -- Prefer to use the Soromid utilities
   params.prefer["Bio-Neural Combat AI"]  = 100
   params.prefer["Nexus Stealth Coating"] = 100
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = soromid_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   local cores
   local srmcor = soromid_cores[ sname ]
   if srmcor then
      cores = srmcor()
   else
      cores = ecores.get( p, { all="elite" } )
   end

   -- Try to equip
   return optimize.optimize( p, cores, soromid_outfits, params )
   -- TODO randomly change some bio plasma weapons to weaker ones
end

return equip_soromid

