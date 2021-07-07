local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local zalek_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Za'lek Light Drone Fighter Dock", "Za'lek Heavy Drone Fighter Dock",
   "Za'lek Bomber Drone Fighter Dock",
   "Ragnarok Beam", "Grave Beam",
   -- Medium Weapons
   "Za'lek Heavy Drone Fighter Bay", "Za'lek Light Drone Fighter Bay",
   "Za'lek Bomber Drone Fighter Bay",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Enygma Systems Spearhead Launcher", "TeraCom Fury Launcher",
   "TeraCom Headhunter Launcher", "TeraCom Medusa Launcher",
   "TeraCom Vengeance Launcher",
   "Za'lek Hunter Launcher", "Za'lek Reaper Launcher",
   "Grave Lance", "Orion Beam",
   -- Small Weapons
   "Particle Beam", "Particle Lance",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Faraday Tempest Coating", "Hive Combat AI",
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

local zalek_params = {
   ["Za'lek Demon"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
   ["Za'lek Mephisto"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local zalek_cores = {
   ["Za'lek Sting"] = function (p) return {
         choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" },
         "Tricon Cyclone Engine",
         choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" },
      } end,
   ["Za'lek Demon"] = function (p) return {
         choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System" },
         "Tricon Cyclone II Engine",
         choose_one{ "Nexus Medium Stealth Plating", "S&K Medium-Heavy Combat Plating" },
      } end,
   ["Za'lek Mephisto"] = function (p) return {
         "Milspec Orion 9901 Core System",
         choose_one{ "Unicorp Eagle 7000 Engine", "Tricon Typhoon II Engine" },
         choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" },
      } end,
   ["Za'lek Diablo"] = function (p) return {
         "Milspec Thalos 9802 Core System",
         choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" },
         choose_one{ "Tricon Typhoon II Engine", "Melendez Mammoth XL Engine" },
      } end,
   ["Za'lek Hephaestus"] = function (p) return {
         "Milspec Thalos 9802 Core System",
         choose_one{ "Unicorp D-68 Heavy Plating", "S&K Superheavy Combat Plating" },
         "Melendez Mammoth XL Engine",
      } end,
}

--[[
-- @brief Does Za'lek pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_zalek( p, opt_params )
   opt_params = opt_params or {}

   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Za'lekish
   local params = eparams.choose( p )
   -- Prefer to use the Za'lek utilities
   params.prefer["Hive Combat AI"]          = 100
   params.prefer["Faraday Tempest Coating"] = 100
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = zalek_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   local cores
   local zlkcor = zalek_cores[ sname ]
   if zlkcor then
      cores = zlkcor(p)
   else
      cores = ecores.get( p, { all="elite" } )
   end

   -- Try to equip
   return optimize.optimize( p, cores, zalek_outfits, params )
end

return equip_zalek
