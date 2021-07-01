local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'factions.equip.cores'

local sirius_outfits = {
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
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
}

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
function equip( p )
   local ps    = p:ship()
   local pc    = ps:class()
   local sname = ps:nameRaw()

   -- Choose parameters and make Siriusish
   local params = equipopt.params.choose( p )
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

   -- See cores
   local cores
   local srscor = sirius_cores[ sname ]
   if srscor then
      cores = srscor()
   else
      cores = ecores.get( pc, { all="elite" } )
   end

   -- Try to equip
   equipopt.equip( p, cores, sirius_outfits, params )
end
