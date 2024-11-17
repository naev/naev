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
   "Heavy Ripper Cannon", "Laser Turret MK2",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Vengeance Launcher", "Enygma Systems Spearhead Launcher",
   "Unicorp Caesar IV Launcher", "Enygma Systems Turreted Fury Launcher",
   -- Small Weapons
   "TeraCom Banshee Launcher",
   "Laser Cannon MK1", "Ripper Cannon",
   -- Point Defense
   "Guardian Overseer System",
   "Guardian Interception System",
   -- Utility
   "Hunting Combat AI", "Photo-Voltaic Nanobot Coating",
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   -- Heavy Structural
   "Battery IV", "Large Fuel Pod", "Battery III", "Reactor Class III",
   "Shield Capacitor III", "Nanobond Plating",
   "Large Shield Booster",
   -- Medium Structural
   "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
   "Active Plating", "Reactor Class II",
   "Medium Shield Booster",
   -- Small Structural
   "Plasteel Plating", "Battery I", "Improved Stabilizer", "Engine Reroute",
   "Reactor Class I", "Small Fuel Pod",
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
         "Milspec Orion 2301 Core System",
         "Tricon Zephyr Engine",
         choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" },
      } end,
   ["Empire Lancelot"] = function () return {
         "Milspec Orion 3701 Core System",
         "Tricon Zephyr II Engine",
         choose_one{ "Nexus Light Stealth Plating", "S&K Light Combat Plating" },
      } end,
   ["Empire Admonisher"] = function () return {
         "Milspec Orion 4801 Core System",
         "Tricon Cyclone Engine",
         choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" },
      } end,
   ["Empire Pacifier"] = function () return {
         "Milspec Orion 5501 Core System",
         "Tricon Cyclone II Engine",
         "S&K Medium-Heavy Combat Plating",
      } end,
   ["Empire Hawking"] = function () return {
         "Milspec Orion 8601 Core System",
         "Tricon Typhoon Engine",
         "S&K Heavy Combat Plating",
      } end,
   ["Empire Peacemaker"] = function () return {
         "Milspec Orion 9901 Core System",
         "Melendez Mammoth XL Engine",
         "S&K Superheavy Combat Plating",
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
