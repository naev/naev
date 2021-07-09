local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local dvaered_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Railgun", "Repeating Railgun",
   "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Mass Driver", "Turreted Vulcan Gun",
   "Unicorp Caesar IV Launcher", "TeraCom Imperator Launcher",
   -- Small Weapons
   "Shredder",
   "Vulcan Gun", "Gauss Gun",
   "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
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
   ["Dvaered Vigilance"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,1) },
         }
      } end,
   ["Dvaered Goddard"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         }
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
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
   ["Dvaered Goddard"] = function () return {
         "S&K Superheavy Combat Plating",
      } end,
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

   -- Choose parameters and make Za'lekish
   local params = eparams.choose( p )
   -- Prefer to use the Za'lek utilities
   params.prefer["Cyclic Combat AI"] = 100
   params.prefer["Milspec Impacto-Plastic Coating"] = 100
   params.type_range["Bolt Weapon"] = { min = 1 }
   params.max_same_stru = 3
   params.ew = 0 -- Don't care about electronic warfare
   params.min_energy_regen = 1.0 -- Dvaereds want more energy
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = dvaered_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   local cores
   local dvrcor = dvaered_cores[ sname ]
   if dvrcor then
      cores = dvrcor()
   else
      cores = ecores.get( p, { hulls="elite" } )
   end

   -- Try to equip
   return optimize.optimize( p, cores, dvaered_outfits, params )
end

return equip_dvaered
