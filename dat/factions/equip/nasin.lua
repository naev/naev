--more of a dummy file to avoid errors.

-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does sirius pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "military" then
      equip_siriusMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


-- CANNONS
function equip_forwardSrsLow ()
   return { "Razor MK2", "Razor MK3", "Ion Cannon" }
end
function equip_forwardSrsMed ()
   return { "Heavy Ion Cannon", "Razor MK3", }
end
-- TURRETS
function equip_turretSrsLow ()
   return { "Razor Turret MK1" }
end
function equip_turretSrsMed ()
   return { "Razor Turret MK2", "Razor Turret MK3", "Heavy Ion Turret" }
end
function equip_turretSrsHig ()
   return { "Heavy Ion Turret" }
end
-- RANGED
function equip_rangedSrs ()
   return { "Unicorp Headhunter Launcher" }
end
function equip_secondarySrs ()
   return { "Unicorp Headhunter Launcher" }
end



--[[
-- @brief Equips a sirius military type ship.
--]]
function equip_siriusMilitary( p, shipsize )
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()
   local scramble

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   secondary   = { }

   weapons     = {}
   scramble    = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout
      if class == "Scout" then
         equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 3701 Core System", "Schafer & Kane Light Stealth Plating")
         use_primary    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 3701 Core System", "Schafer & Kane Light Stealth Plating")
         primary        = icmb( equip_forwardSrsLow(), equip_forwardSrsMed() )
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()


      -- Bomber
      elseif class == "Bomber" then
         equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 3701 Core System", "Schafer & Kane Light Combat Plating")
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardSrsLow(), use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()

      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Corvette
      if class == "Corvette" then
         equip_cores(p, "Tricon Centaur Mk7 Engine", "Milspec Orion 5501 Core System", "Schafer & Kane Medium Solar Plating")
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsLow() )
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()

      end

      -- Destroyer
      if class == "Destroyer" then
         equip_cores(p, "Tricon Centaur Mk7 Engine", "Milspec Orion 5501 Core System", "Schafer & Kane Medium Combat Plating Gamma")
         scramble       = true
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsMed() )
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()

      end

   else
      -- TODO: Divide into carrier and cruiser classes.
      equip_cores(p, "Tricon Harpy Mk11 Engine", "Milspec Orion 9901 Core System", "Schafer & Kane Heavy Combat Plating Gamma")
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( equip_turretSrsHig(), use_primary )
      addWeapons( equip_secondarySrs(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()

   end

   equip_ship( p, scramble, weapons, medium, low,
               use_medium, use_low )
end
