local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local thurion_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Heavy Laser Turret", "Ragnarok Beam",
   "Thurion Perspicacity Fighter Bay",
   "Heavy Ripper Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
   "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher", "Convulsion Launcher",
   "Enygma Systems Turreted Fury Launcher",
   "Turreted Convulsion Launcher",
   "Laser Turret MK2", "Razor Turret MK2", "Orion Beam",
   "EMP Grenade Launcher", "Enygma Systems Turreted Fury Launcher",
   -- Small Weapons
   "Ripper Cannon", "Slicer", "Laser Cannon MK2", "Razor MK2",
   "Laser Cannon MK1", "Razor MK1", "TeraCom Mace Launcher",
   "TeraCom Banshee Launcher", "Electron Burst Cannon",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Nebula Resistant Coating",
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
   "Adaptive Stealth Plating",
   "Small Shield Booster",
}}

local thurion_params = {
   --["Sirius Demon"] = function () return {
   ["Thurion Apprehension"] = function () return {
         type_range = {
            ["Launcher"] = { max = 1 },
         },
      } end,
   ["Thurion Certitude"] = function  () return {
         type_range = {
            ["Launcher"] = { max = 0 },
         },
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local thurion_cores = {
}

--[[
-- @brief Does Thurion pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_thurion( p, opt_params )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Thurion-ish
   local params = eparams.choose( p )
   -- Prefer to use the Thurion utilities
   params.prefer["Nebula Resistant Coating"] = 100
   params.max_same_util = 3
   params.max_same_stru = 3
   params.max_same_weap = 3
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = thurion_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   cores = ecores.get( p, { all="elite" } )

   -- Try to equip
   return optimize.optimize( p, cores, thurion_outfits, params )
end

return equip_thurion
