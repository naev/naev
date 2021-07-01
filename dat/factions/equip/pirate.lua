local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'factions.equip.cores'

local pirate_outfits = {
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
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
}

local pirate_params = {
   ["Pirate Demon"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
}
local pirate_cores = {
   ["Hyena"] = function () return ecores.get( "Fighter", { all={ "elite" }, heavy=false } ) end,
   ["Pirate Shark"] = function () return ecores.get( "Fighter", { all={ "elite" }, heavy=false } ) end,
   --[[
         choose_one{ "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System", },
         choose_one{ "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating", },
         choose_one{ "Nexus Dart 150 Engine", "Tricon Zephyr Engine", },
   --]]
   ["Pirate Vendetta"] = function () return ecores.get( "Fighter", { all={ "elite" } } ) end,
   --[[
         choose_one{ "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System", },
         choose_one{ "Unicorp Hawk 350 Engine", "Melendez Ox XL Engine", "Tricon Zephyr II Engine", },
         choose_one{ "Unicorp D-2 Light Plating", "Unicorp D-4 Light Plating", },
   --]]
   ["Pirate Ancestor"] = function () return {
      } end,
   ["Lancelot"] = function () return {
         choose_one{ "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System", "Milspec Thalos 3602 Core System", },
      } end,
   ["Pirate Phalanx"] = function () return {
         choose_one{ "Unicorp PT-200 Core System", "Milspec Orion 4801 Core System", },
      } end,
   ["Pirate Admonisher"] = function () return {
         choose_one{ "Unicorp PT-200 Core System", "Milspec Orion 4801 Core System", },
      } end,
   ["Pacifier"] = function () return {
         choose_one{ "Unicorp PT-310 Core System", "Milspec Orion 5501 Core System", },
      } end,
   ["Pirate Kestrel"] = function () return {
         choose_one{ "Unicorp PT-2200 Core System", "Milspec Orion 8601 Core System", },
         choose_one{ "Nexus Bolt 4500 Engine", "Krain Remige Engine", "Tricon Typhoon Engine", },
      } end,
   ["Pirate Rhino"] = function () return {
      } end,
}

--[[
-- @brief Does Pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   local ps = p:ship()
   local pc = ps:class()
   local sname = ps:nameRaw()

   -- Choose parameters and make Pirateish
   local params = equipopt.params.choose( p )
   -- Prefer to use the Pirate utilities
   params.prefer["Scanning Combat AI"]      = 100
   params.max_same_stru = 1
   params.max_same_util = 1
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

   -- See cores
   local cores = pirate_cores[ sname ]() or ecores.get( pc, { all={ "elite" } } )

   -- Try to equip
   equipopt.equip( p, cores, pirate_outfits, params )
end
