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


function equip_forwardPirLow ()
   return { "Plasma Blaster", "Plasma Blaster", "Laser Cannon MK1" }
end
function equip_forwardPirMed ()
   return { "Plasma Blaster MK2", "Plasma Blaster MK2", "Laser Cannon MK2" }
end
function equip_secondaryPirLow ()
   return { "Seeker Launcher" }
end
function equip_secondaryPirMedLow ()
   return { "Seeker Launcher", "Banshee Launcher" }
end
function equip_turretPir ()
   return { "Laser Turret MK2", "Heavy Ion Turret" }
end


--[[
-- @brief Equips a pirate military type ship.
--]]
function equip_pirateMilitary( p, shipsize )
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Civilian Jammer" }
   secondary   = { }
   apu         = { }

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout - shouldn't exist
      if class == "Scout" then
         primary        = equip_forwardPirLow ()
         use_primary    = rnd.rnd(1,#nhigh)
         medium         = { "Generic Afterburner", "Milspec Jammer" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         primary        = equip_forwardPirLow()
         if nhigh > 3 then
            use_primary    = nhigh-1
            secondary      = equip_secondaryPirLow()
            use_secondary  = 1
         else
            use_primary    = nhigh
         end
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         primary        = equip_forwardPirLow()
         secondary      = equip_secondaryPirLow()
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      primary        = equip_forwardPirMed()
      secondary      = equip_secondaryPirMedLow()
      use_secondary  = rnd.rnd(1,2)
      use_primary    = nhigh - use_secondary
      medium         = equip_mediumMed()
      low            = equip_lowMed()
      apu            = equip_apuMed()

   else
      primary        = equip_turretPir()
      secondary      = equip_secondaryPirMedLow()
      use_primary    = nhigh-2
      use_secondary  = 2
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low )
end



