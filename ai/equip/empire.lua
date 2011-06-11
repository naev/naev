--[[
-- @brief Does empire pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_empire( p )
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
   return { "Headhunter Launcher" }
end
function equip_secondaryEmp ()
   return { "Headhunter Launcher" }
end



--[[
-- @brief Equips a empire military type ship.
--]]
function equip_empireMilitary( p, shipsize )
   local medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()
   local scramble

   -- Defaults
   medium      = { "Civilian Jammer" }
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
         medium         = { "Generic Afterburner", "Milspec Jammer" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( equip_forwardEmpMed(), use_primary )
         addWeapons( equip_secondaryEmp(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardEmpLow(), use_primary )
         addWeapons( equip_rangedEmp(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Corvette
      if class == "Corvette" then
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( equip_forwardEmpMed(), use_primary )
         addWeapons( equip_secondaryEmp(), use_secondary )
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

      -- Destroyer
      if class == "Destroyer" then
         use_secondary  = rnd.rnd(1,2)
         use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
         use_forward    = nhigh - use_turrets
         addWeapons( equip_secondaryEmp(), use_secondary )
         addWeapons( equip_turretEmpMed(), use_turrets )
         addWeapons( equip_forwardEmpMed(), use_forward )
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

   else
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( equip_turretEmpHig(), use_primary )
      addWeapons( equip_secondaryEmp(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end

   equip_ship( p, scramble, weapons, medium, low, apu,
               use_medium, use_low )
end
