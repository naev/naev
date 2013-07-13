-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does empire pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "military" then
      equip_empireMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


-- CANNONS
function equip_forwardEmpLow ()
   return { "Laser Cannon MK2", "Laser Cannon MK3" }
end
function equip_forwardEmpMed ()
   return { "Laser Cannon MK3", "Ripper Cannon", "Heavy Ripper Cannon" }
end
-- TURRETS
function equip_turretEmpLow ()
   return { "Laser Turret MK2" }
end
function equip_turretEmpMed ()
   return { "Laser Turret MK2", "Laser Turret MK3" }
end
function equip_turretEmpHig ()
   return { "Heavy Laser", "Turbolaser" }
end
-- RANGED
function equip_rangedEmp ()
   return { "Unicorp Headhunter Launcher" }
end
function equip_secondaryEmp ()
   return { "Unicorp Headhunter Launcher" }
end



--[[
-- @brief Equips a empire military type ship.
--]]
function equip_empireMilitary( p, shipsize )
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
         equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 3702 Core System", "Schafer & Kane Light Stealth Plating")
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( equip_forwardEmpMed(), use_primary )
         addWeapons( equip_secondaryEmp(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()


      -- Bomber
      elseif class == "Bomber" then
         equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 3702 Core System", "Schafer & Kane Light Combat Plating")
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardEmpLow(), use_primary )
         addWeapons( equip_rangedEmp(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()

      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Corvette
      if class == "Corvette" then
         equip_cores(p, "Tricon Centaur Mk7 Engine", "Milspec Orion 5502 Core System", "Schafer & Kane Medium Solar Plating")
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( equip_forwardEmpMed(), use_primary )
         addWeapons( equip_secondaryEmp(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()

      end

      -- Destroyer
      if class == "Destroyer" then
         equip_cores(p, "Tricon Centaur Mk7 Engine", "Milspec Orion 5502 Core System", "Schafer & Kane Medium Solar Plating")
         use_secondary  = rnd.rnd(1,2)
         use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
         use_forward    = nhigh - use_secondary - use_turrets
         addWeapons( equip_secondaryEmp(), use_secondary )
         addWeapons( equip_turretEmpMed(), use_turrets )
         addWeapons( equip_forwardEmpMed(), use_forward )
         medium         = equip_mediumMed()
         low            = equip_lowMed()

      end

   else -- "large"
      -- TODO: Divide into carrier and cruiser classes.
      equip_cores(p, "Tricon Harpy Mk11 Engine", "Milspec Orion 9903 Core System", "Schafer & Kane Heavy Solar Plating")
      use_secondary  = 2
      if rnd.rnd() > 0.4 then -- Anti-fighter variant.
         use_turrets    = nhigh - use_secondary - rnd.rnd(2,3)
         use_medturrets = nhigh - use_secondary - use_turrets
         addWeapons( equip_turretEmpMed(), use_medturrets )
      else -- Anti-capital variant.
         use_turrets    = nhigh - use_secondary
      end
      addWeapons( equip_turretEmpHig(), use_turrets )
      addWeapons( equip_secondaryEmp(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()

   end

   equip_ship( p, scramble, weapons, medium, low,
               use_medium, use_low )
end
