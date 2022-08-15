local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

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
   "Shredder", "Vulcan Gun", "Gauss Gun",
   "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
   -- Utility
   "Cyclic Combat AI", "Milspec Impacto-Plastic Coating",
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   -- Heavy Structural
   "Large Fuel Pod", "Battery III", "Reactor Class III",
   "Shield Capacitor III", "Nanobond Plating", "Biometal Armour",
   -- Medium Structural
   "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
   "Active Plating", "Reactor Class II",
   -- Small Structural
   "Plasteel Plating", "Battery I", "Improved Stabilizer", "Engine Reroute",
   "Reactor Class I", "Small Fuel Pod",
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
         "S&K Light Combat Plating",
      } end,
   ["Dvaered Ancestor"] = function () return {
         "S&K Light Combat Plating",
      } end,
   ["Dvaered Phalanx"] = function () return {
         "S&K Medium Combat Plating",
      } end,
   ["Dvaered Vigilance"] = function () return {
         "S&K Medium-Heavy Combat Plating",
      } end,
   ["Dvaered Retribution"] = function () return {
         "S&K Heavy Combat Plating",
      } end,
   ["Dvaered Goddard"] = function () return {
         "S&K Superheavy Combat Plating",
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
   min_energy_regen = 1.0, -- Davereds want more energy
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
   params = tmerge( params, opt_params )

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
