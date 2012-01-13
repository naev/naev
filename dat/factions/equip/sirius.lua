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
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()
   local scramble

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   secondary   = { }
   apu         = { }
   weapons     = {}
   scramble    = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout
      if class == "Scout" then
         use_primary    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         primary        = icmb( equip_forwardSrsLow(), equip_forwardSrsMed() )
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardSrsLow(), use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Corvette
      if class == "Corvette" then
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsLow() )
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

      -- Destroyer
      if class == "Destroyer" then
         scramble       = true
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsMed() )
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( primary, use_primary )
         addWeapons( equip_secondarySrs(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

   else
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( equip_turretSrsHig(), use_primary )
      addWeapons( equip_secondarySrs(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end

   equip_ship( p, scramble, weapons, medium, low, apu,
               use_medium, use_low )
end
