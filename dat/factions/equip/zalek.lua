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
   ["Za'lek Demon"] = function ()
      return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      }
   end,
   ["Za'lek Mephisto"] = function ()
      return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      }
   end,
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
   params = equipopt.params.choose( p )
   -- Prefer to use the Za'lek utilities
   params.prefer = {
      ["Hive Combat AI"]          = 100,
      ["Faraday Tempest Coating"] = 100,
   }
   -- Per ship tweaks
   local sp = zalek_params[ sname ]
   if sp then
      for k,v in pairs(sp()) do
         params[k] = v
      end
   end

   -- Try to equip
   equipopt.equip( p, cores, zalek_outfits, params )
end
