require "factions/equip/generic"

equip_typeOutfits_weapons["Derivative"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
   },
   {
      "Laser Cannon MK1", "Plasma Blaster MK1",
   }
}
equip_typeOutfits_weapons["Kahan"] = {
   {
      num = 2;
      "Railgun", "Heavy Razor Turret", "Grave Beam",
   },
   {
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   }
}
equip_typeOutfits_weapons["Archimedes"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   },
   {
      "Heavy Razor Turret", "Grave Beam",
   },
}
equip_typeOutfits_weapons["Watson"] = {
   {
      num = 2;
      "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
   },
   {
      "Proteron Derivative Fighter Bay",
   },
   {
      "Heavy Razor Turret", "Grave Beam",
   },
}

local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'factions.equip.cores'

local proteron_outfits = {
   -- Heavy Weapons
   "Proteron Derivative Fighter Bay",
   "Heavy Razor Turret", "Grave Beam", "Railgun",
   "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
   "Heavy Ripper Turret",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   -- Small Weapons
   "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
   "Laser Cannon MK1", "Plasma Blaster MK1",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
}

local proteron_params = {
   --["Proteron Demon"] = function () return {
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

--[[
-- @brief Does Proteron pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Proteronish
   local params = equipopt.params.choose( p )
   -- Prefer to use the Proteron utilities
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = proteron_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end

   -- See cores
   cores = ecores.get( p, { all="elite" } )

   -- Try to equip
   equipopt.equip( p, cores, proteron_outfits, params )
end
