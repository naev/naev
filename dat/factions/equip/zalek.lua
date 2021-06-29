equipopt = require 'equipopt'

local zalek_outfits = {
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
   "Particle Beam",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Faraday Tempest Coating", "Hive Combat AI",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery", "Shield Capacitor", "Reactor Class I",
}

local zalek_skip = {
   ["Za'lek Scout Drone"]  = true,
   ["Za'lek Light Drone"]  = true,
   ["Za'lek Heavy Drone"]  = true,
   ["Za'lek Bomber Drone"] = true,
}

local zalek_params = {
   ["Za'lek Demon"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
         weap = 2,
      } end,
   ["Za'lek Mephisto"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
         weap = 2,
         min_energy_regen = 0,
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local zalek_cores = {
   ["Za'lek Sting"] = function () return {
         choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" },
         "Tricon Cyclone Engine",
         choose_one{ "Nexus Medium Stealth Plating", "S&K Medium Combat Plating" },
      } end,
   ["Za'lek Demon"] = function () return {
         choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System" },
         "Tricon Cyclone II Engine",
         choose_one{ "Nexus Medium Stealth Plating", "S&K Medium-Heavy Combat Plating" },
      } end,
   ["Za'lek Mephisto"] = function () return {
         "Milspec Orion 9901 Core System",
         choose_one{ "Unicorp Eagle 7000 Engine", "Tricon Typhoon II Engine" },
         choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" },
      } end,
   ["Za'lek Diablo"] = function () return {
         "Milspec Thalos 9802 Core System",
         choose_one{ "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating" },
         "Tricon Typhoon II Engine",
      } end,
   ["Za'lek Hephaestus"] = function () return {
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
function equip( p )
   local sname = p:ship():nameRaw()
   if zalek_skip[sname] then return end

   -- Choose parameters and make Za'lekish
   local params = equipopt.params.choose( p )
   -- Prefer to use the Za'lek utilities
   params.prefer["Hive Combat AI"]          = 100
   params.prefer["Faraday Tempest Coating"] = 100
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = zalek_params[ sname ]
   if sp then
      for k,v in pairs(sp()) do
         if type(v)=="table" then
            for i,e in pairs(v) do
               params[k][i] = e
            end
         else
            params[k] = v
         end
      end
   end

   -- See cores
   local cores = zalek_cores[ sname ]()

   -- Try to equip
   equipopt.equip( p, cores, zalek_outfits, params )
end
