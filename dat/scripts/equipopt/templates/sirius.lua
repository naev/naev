local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local sirius_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Fidelity Fighter Bay",
   "Heavy Razor Turret", "Ragnarok Beam",
   "Heavy Ion Turret", "Grave Beam",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
   "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   -- Small Weapons
   "Slicer", "Razor MK2", "Ion Cannon",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Pinpoint Combat AI", "Lattice Thermal Coating",
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

local sirius_params = {
   --["Sirius Demon"] = function () return {
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local sirius_cores = {
}

--[[
-- @brief Does Sirius pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_sirius( p, opt_params )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Siriusish
   local params = eparams.choose( p )
   -- Prefer to use the Sirius utilities
   params.prefer["Pinpoint Combat AI"]      = 100
   params.prefer["Lattice Thermal Coating"] = 100
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = sirius_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   local cores
   local srscor = sirius_cores[ sname ]
   if srscor then
      cores = srscor()
   else
      cores = ecores.get( p, { all="elite" } )
   end

   -- Try to equip
   return optimize.optimize( p, cores, sirius_outfits, params )
end

return equip_sirius
