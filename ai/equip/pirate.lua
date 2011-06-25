--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_pirate( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )
   local class = p:ship():class()

   -- Split by type
   if shiptype == "military" or class == "Armoured Transport" then
      equip_pirateMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


--[[
   The idea is they get the basic stuff + the middle stuff a tier lower. For example:
      low = low basic weapons
      med = low licensed weapons + medium basic weapons
      hig = medium licensed weapons + high basic weapons
--]]
function equip_forwardPirLow ()
   return { "Laser Cannon MK1", "Ion Cannon", "Razor MK1", "Gauss Gun" }
end
function equip_forwardPirMed ()
   return { "Laser Cannon MK2", "Ripper Cannon", "Razor MK2", "Vulcan Gun", "Mass Driver MK1" }
end
function equip_forwardPirHig ()
   return { "Heavy Ion Cannon", "Mass Driver MK2" }
end
function equip_turretPirLow ()
   return { "Razor Turret MK1", "Turreted Gauss Gun" }
end
function equip_turretPirMed ()
   return { "Laser Turret MK1" }
end
function equip_turretPirMed ()
   return { "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun" }
end
function equip_turretPirHig ()
   return { "Railgun Turret", "Heavy Laser" }
end
function equip_secondaryPirLow ()
   return { "Unicorp Fury Launcher" }
end
function equip_secondaryPirMedLow ()
   return { "Unicorp Fury Launcher", "Unicorp Banshee Launcher" }
end


--[[
-- @brief Equips a pirate military type ship.
--]]
function equip_pirateMilitary( p, shipsize )
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   apu         = { }
   weapons     = {}

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout - shouldn't exist
      if class == "Scout" then
         use_primary    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardPirLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         if nhigh > 3 then
            use_primary    = nhigh-1
            use_secondary  = 1
            addWeapons( equip_secondaryPirLow(), use_secondary )
         else
            use_primary    = nhigh
         end
         addWeapons( equip_forwardPirLow(), use_primary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardPirLow(), use_primary )
         addWeapons( equip_secondaryPirLow(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()

      use_secondary  = rnd.rnd(1,2)
      use_primary    = nhigh - use_secondary

      -- Corvette
      if class == "Corvette" then
         addWeapons( equip_turretPirLow(), use_secondary )
      else
         addWeapons( equip_turretPirMed(), use_secondary )
      end
      addWeapons( equip_forwardPirMed(), use_primary )
      medium         = equip_mediumMed()
      low            = equip_lowMed()
      apu            = equip_apuMed()

   else
      primary        = icmb( equip_turretPirHig(), equip_turretPirMed() )
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( primary, use_primary )
      addWeapons( equip_secondaryPirMedLow(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end
   equip_ship( p, true, weapons, medium, low, apu,
               use_medium, use_low )
end



