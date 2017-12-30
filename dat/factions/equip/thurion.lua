-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does thurion pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Start with an empty ship
   p:rmOutfit("all")
   p:rmOutfit("cores")

   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "military" then
      equip_thurionMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


-- CANNONS
function equip_forwardThrLow ()
   return { "Laser Cannon MK3", "Ion Cannon" }
end
function equip_forwardThrMed ()
   return { "Ripper Cannon", "Heavy Ion Cannon" }
end
-- TURRETS
function equip_turretThrLow ()
   return { "Laser Turret MK2", "Turreted Convulsion Launcher" }
end
function equip_turretThrMed ()
   return { "Laser Turret MK3", "Heavy Ion Turret", "Turreted Convulsion Launcher" }
end
function equip_turretThrMedHig ()
   return { "Heavy Ripper Turret" }
end
function equip_turretThrHig ()
   return { "Heavy Laser", "Turbolaser" }
end
-- RANGED
function equip_rangedThr ()
   return { "Electron Burst Cannon", "Convulsion Launcher", "Enygma Systems Spearhead Launcher" }
end
function equip_secondaryThr ()
   return { "Convulsion Launcher", "Enygma Systems Spearhead Launcher" }
end
-- NON-COMBAT
--[[
-- Utility slots
--]]
function equip_mediumThrLow ()
   return { "Thurion Reactor Class I", "Unicorp Scrambler", "Small Shield Booster", "Sensor Array" }
end
function equip_mediumThrMed ()
   return { "Thurion Reactor Class II", "Milspec Scrambler", "Medium Shield Booster" }
end
function equip_mediumThrHig ()
   return { "Thurion Reactor Class III", "Milspec Scrambler", "Large Shield Booster" }
end

--[[
-- Structure slots
--]]
function equip_lowThrLow ()
   return { "Battery", "Shield Capacitor", "Engine Reroute", "Adaptive Stealth Plating" }
end
function equip_lowThrMed ()
   return { "Shield Capacitor II", "Thurion Engine Reroute", "Battery II", "Active Plating" }
end
function equip_lowThrHig ()
   return { "Shield Capacitor III", "Shield Capacitor IV", "Battery III" }
end



--[[
-- @brief Equips a thurion military type ship.
--]]
function equip_thurionMilitary( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local use_forward, use_turrets, use_medturrets
   local nhigh, nmedium, nlow = p:ship():slots()
   local scramble

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   weapons     = {}
   scramble    = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()
      cores = {
         {"Tricon Zephyr Engine", "Milspec Hermes 2202 Core System", "S&K Ultralight Combat Plating"},
         {"Tricon Zephyr II Engine", "Milspec Hermes 3602 Core System", "S&K Light Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      -- Scout
      if class == "Scout" then
         equip_cores(p, "Tricon Zephyr Engine", "Milspec Hermes 2202 Core System", "S&K Ultraight Stealth Plating")
         use_primary    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( equip_forwardThrLow(), use_primary )
         addWeapons( equip_secondaryThr(), use_secondary )
         medium         = equip_mediumThrLow()
         low            = equip_lowThrLow()


      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(0,1)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardThrLow(), use_primary )
         addWeapons( equip_rangedThr(), use_secondary )
         medium         = equip_mediumThrLow()
         low            = equip_lowThrLow()

      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      cores = {
         {"Tricon Cyclone Engine", "Milspec Hermes 4702 Core System", "S&K Medium Stealth Plating"},
         {"Tricon Cyclone II Engine", "Milspec Hermes 5402 Core System", "S&K Medium-Heavy Stealth Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))
      
      -- Corvette
      if class == "Corvette" then
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( equip_forwardThrMed(), use_primary )
         addWeapons( equip_secondaryThr(), use_secondary )
         medium         = equip_mediumThrMed()
         low            = equip_lowThrMed()

      end

      -- Destroyer
      if class == "Destroyer" then
         use_secondary  = rnd.rnd(1,2)
         use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
         use_forward    = nhigh - use_secondary - use_turrets
         addWeapons( equip_secondaryThr(), use_secondary )
         addWeapons( icmb( equip_turretThrMed(), equip_turretThrMedHig() ), use_turrets )
         addWeapons( equip_forwardThrMed(), use_forward )
         medium         = equip_mediumThrMed()
         low            = equip_lowThrMed()

      end

   else -- "large"
      -- TODO: Divide into carrier and cruiser classes.
      cores = {
         {"Tricon Typhoon Engine", "Milspec Hermes 8502 Core System", "S&K Heavy Combat Plating"},
         {"Tricon Typhoon II Engine", "Milspec Hermes 9802 Core System", "S&K Superheavy Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      use_secondary  = 2
      if rnd.rnd() > 0.4 then -- Anti-fighter variant.
         use_turrets    = nhigh - use_secondary - rnd.rnd(2,3)
         use_medturrets = nhigh - use_secondary - use_turrets
         addWeapons( equip_turretThrMed(), use_medturrets )
      else -- Anti-capital variant.
         use_turrets    = nhigh - use_secondary
      end
      addWeapons( equip_turretThrHig(), use_turrets )
      addWeapons( equip_secondaryThr(), use_secondary )
      medium         = equip_mediumThrHig()
      low            = equip_lowThrHig()

   end

   equip_ship( p, scramble, weapons, medium, low,
               use_medium, use_low )
end
