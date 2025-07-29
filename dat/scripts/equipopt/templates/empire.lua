local optimize = require 'equipopt.optimize'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'

local empire_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Empire Lancelot Bay",
   "Turbolaser", "Heavy Ripper Turret",
   "Heavy Laser Turret",
   -- Medium Weapons
   "Heavy Ripper Cannon", "Laser Turret MK2", "Heavy Ion Cannon",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Vengeance Launcher", "Enygma Systems Spearhead Launcher",
   "Unicorp Caesar IV Launcher", "Enygma Systems Turreted Fury Launcher",
   -- Small Weapons
   "TeraCom Banshee Launcher", "Unicorp Storm Launcher", "Ion Cannon",
   "Laser Cannon MK1", "Laser Cannon MK2", "Ripper Cannon",
   -- Point Defense
   "Guardian Overseer System",
   "Guardian Interception System",
   -- Utility
   "Hunting Combat AI", "Photo-Voltaic Nanobot Coating",
   "Unicorp Scrambler", "Unicorp Light Afterburner", "Milspec Jammer",
   "Sensor Array", "Emergency Shield Booster", "Milspec Scrambler",
   "Unicorp Medium Afterburner", "Droid Repair Crew", "Unicorp Jammer",
   -- Heavy Structural
   "Battery IV", "Large Fuel Pod", "Battery III", "Reactor Class III",
   "Shield Capacitor III", "Nanobond Plating", "Auxiliary Processing Unit III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II",
   "Active Plating", "Reactor Class II", "Auxiliary Processing Unit II",
   "Medium Shield Booster", "Microbond Plating",
   -- Small Structural
   "Plasteel Plating", "Battery I", "Improved Stabilizer", "Engine Reroute",
   "Reactor Class I", "Auxiliary Processing Unit I",
   "Small Shield Booster",
}}
local empire_outfits_collective = eoutfits.merge{
   empire_outfits,
   { "Drone Bay" },
}


local empire_params = {
   ["Empire Lancelot"] = function () return {
         type_range = {
            ["Launcher"] = { min = 1 },
         },
      } end,
   ["Empire Peacemaker"] = function () return {
         ["turret"] = 2,
      } end,
}

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local empire_cores = {
   ["Empire Shark"] = function () return {
         systems = "Milspec Orion 2301 Core System",
         engines = choose_one{ "Tricon Zephyr Engine", "Nexus Dart 160 Engine", "Melendez Ox Engine" },
         hull = choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" },
      } end,
   ["Empire Lancelot"] = function () return {
         systems = "Milspec Orion 2301 Core System",
         systems_secondary = "Milspec Orion 2301 Core System",
         engines = choose_one{ "Tricon Zephyr Engine", "Nexus Dart 160 Engine", "Melendez Ox Engine" },
         engines_secondary  = choose_one{ "Tricon Zephyr Engine", "Nexus Dart 160 Engine", "Melendez Ox Engine" },
         hull = choose_one{ "Nexus Shadow Weave", "S&K Skirmish Plating" },
         hull_secondary = "S&K Skirmish Plating",
      } end,
   ["Empire Admonisher"] = function () return {
         systems = "Milspec Orion 4801 Core System",
         engines = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
      } end,
   ["Empire Pacifier"] = function () return {
         systems = "Milspec Orion 4801 Core System",
         systems_secondary = "Milspec Orion 4801 Core System",
         engines = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         engines_secondary = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
         hull_secondary = "S&K Battle Plating",
      } end,
   ["Empire Hawking"] = function () return {
         systems = "Milspec Orion 8601 Core System",
         engines = choose_one{ "Tricon Typhoon Engine", "Nexus Bolt 3000 Engine", "Melendez Mammoth Engine"},
         hull = "S&K War Plating",
      } end,
   ["Empire Peacemaker"] = function () return {
         systems = "Milspec Orion 8601 Core System",
         systems_secondary = "Milspec Orion 8601 Core System",
         engines = "Melendez Mammoth Engine",
         engines_secondary = choose_one{ "Melendez Mammoth Engine", "Nexus Bolt 3000 Engine" },
         hull = "S&K War Plating",
         hull_secondary = "S&K War Plating",
      } end,
}

local empire_params_overwrite = {
   -- Prefer to use the Empire utilities
   prefer = {
      ["Hunting Combat AI"] = 100,
      ["Photo-Voltaic Nanobot Coating"] = 100,
   },
   type_range = {
      ["Bolt Weapon"] = { min = 1 },
      ["Launcher"] = { max = 2 },
   },
   max_same_stru = 3,
   bolt = 1.5,
   min_energy_regen = 1.0, -- Empire loves energy
}

--[[
-- @brief Does Empire pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_empire( p, opt_params )
   opt_params = opt_params or {}
   local emp_out
   if diff.isApplied( "collective_dead" ) then
      emp_out = empire_outfits_collective
   else
      emp_out = empire_outfits
   end
   if opt_params.outfits_add then
      emp_out = eoutfits.merge{ emp_out, opt_params.outfits_add }
   end

   local sname = p:ship():nameRaw()
   --if empire_skip[sname] then return end

   -- Choose parameters and make Empire-ish
   local params = eparams.choose( p, empire_params_overwrite )
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = empire_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge_r( params, opt_params )

   -- See cores
   local cores = opt_params.cores
   if not cores then
      local empcor = empire_cores[ sname ]
      if empcor then
         cores = empcor()
      else
         cores = ecores.get( p, { all="elite" } )
      end
   end

   -- Set some pilot meta-data
   local mem = p:memory()
   mem.equip = { type="empire", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, emp_out, params )
end

return equip_empire
