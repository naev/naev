local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local dvaered_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Railgun", "Repeating Railgun", "Railgun Turret",
   "Super-Fast Collider Launcher",
   -- Medium Weapons
   "Mass Driver", "Turreted Vulcan Gun",
   "Repeating Banshee Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "Enygma Systems Turreted Fury Launcher", "Enygma Systems Turreted Headhunter Launcher",
   -- Small Weapons
   "Shredder", "Vulcan Gun", "Gauss Gun", "Turreted Gauss Gun",
   "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
   -- Point Defense
   "Dvaered Flare Battery",
   "Ratchet Point Defense",
   -- Utility
   "Cyclic Combat AI", "Milspec Impacto-Plastic Coating",
   "Unicorp Scrambler", "Milspec Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Emergency Shield Booster", "Unicorp Jammer",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   -- Heavy Structural
   "Battery III", "Battery IV", "Reactor Class III",
   "Shield Capacitor III", "Nanobond Plating",
   -- Medium Structural
   "Battery II", "Shield Capacitor II",
   "Active Plating", "Microbond Plating", "Reactor Class II",
   -- Small Structural
   "Plasteel Plating", "Battery I", "Improved Stabilizer",
   "Reactor Class I", "Engine Reroute",
}}

local dvaered_params = {
   ["Dvaered Vendetta"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,4) },
         }
      } end,
   ["Dvaered Phalanx"] = function () return {
         turret = 1.25,
         type_range = {
            ["Launcher"] = { max = rnd.rnd(2,3) },
         }
      } end,
   ["Dvaered Vigilance"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,1) },
         }
      } end,
   ["Dvaered Retribution"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         }
      } end,
   ["Dvaered Goddard"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         }
      } end,
}
--local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local dvaered_cores = { -- Basically elite hulls excluding stealth
   ["Dvaered Vendetta"] = function () return {
         systems = choose_one{ "Milspec Orion 2301 Core System", "Milspec Prometheus 2203 Core System" },
         systems_secondary = choose_one{ "Milspec Orion 2301 Core System", "Milspec Prometheus 2203 Core System" },
         hull = "S&K Skirmish Plating",
         hull_secondary = "S&K Skirmish Plating",
      } end,
   ["Dvaered Ancestor"] = function () return {
         systems = choose_one{ "Milspec Orion 2301 Core System", "Milspec Prometheus 2203 Core System" },
         systems_secondary = choose_one{ "Milspec Orion 2301 Core System", "Milspec Prometheus 2203 Core System" },
         hull = "S&K Skirmish Plating",
         hull_secondary = "S&K Skirmish Plating",
      } end,
   ["Dvaered Phalanx"] = function () return {
         systems = choose_one{ "Milspec Orion 4801 Core System", "Milspec Prometheus 4703 Core System" },
         hull = "S&K Battle Plating",
      } end,
   ["Dvaered Vigilance"] = function () return {
         systems = choose_one{ "Milspec Orion 4801 Core System", "Milspec Prometheus 4703 Core System" },
         systems_secondary = choose_one{ "Milspec Orion 4801 Core System", "Milspec Prometheus 4703 Core System" },
         hull = "S&K Battle Plating",
         hull_secondary = "S&K Battle Plating",
      } end,
   ["Dvaered Retribution"] = function () return {
         systems = choose_one{ "Milspec Orion 8601 Core System", "Milspec Prometheus 8503 Core System" },
         hull = "S&K War Plating",
      } end,
   ["Dvaered Goddard"] = function () return {
         systems = choose_one{ "Milspec Orion 8601 Core System", "Milspec Prometheus 8503 Core System" },
         systems_secondary = choose_one{ "Milspec Orion 8601 Core System", "Milspec Prometheus 8503 Core System" },
         hull = "S&K War Plating",
         hull_secondary = "S&K War Plating",
      } end,
}

local dvaered_params_overwrite = {
   -- Prefer to use the Dvaered utilities
   prefer = {
      ["Cyclic Combat AI"] = 100,
      ["Milspec Impacto-Plastic Coating"] = 100,
   },
   type_range = {
      ["Bolt Weapon"] = { min = 1 },
   },
   max_same_stru = 3,
   ew = 0, -- Don't care about electronic warfare
   min_energy_regen = 1.0, -- Dvaereds want more energy
}

--[[
-- @brief Does Dvaered pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_dvaered( p, opt_params )
   opt_params = opt_params or {}
   local sname = p:ship():nameRaw()
   --if dvaered_skip[sname] then return end

   -- Choose parameters and make Dvaeredish
   local params = eparams.choose( p, dvaered_params_overwrite )
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   params.turret = 0.75 -- They like forwards
   -- Per ship tweaks
   local sp = dvaered_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge_r( params, opt_params )

   -- Outfits
   local outfits = dvaered_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- See cores
   local cores = opt_params.cores
   if not cores then
      local dvrcor = dvaered_cores[ sname ]
      if dvrcor then
         cores = dvrcor()
      else
         cores = ecores.get( p, { hulls="elite" } )
      end
   end

   -- Set some pilot meta-data
   local mem = p:memory()
   mem.equip = { type="dvaered", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_dvaered
