--[[
-- @brief Does sirius pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_sirius( p )
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
   return { "Headhunter Launcher" }
end
function equip_secondarySrs ()
   return { "Headhunter Launcher" }
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
   medium      = { "Civilian Jammer" }
   secondary   = { }
   apu         = { }
   scramble    = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout
      if class == "Scout" then
         primary        = equip_forwardLow ()
         use_primary    = rnd.rnd(1,#nhigh)
         medium         = { "Generic Afterburner", "Milspec Jammer" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         primary        = icmb( equip_forwardSrsLow(), equip_forwardSrsMed() )
         use_primary    = nhigh-1
         secondary      = equip_secondarySrs()
         use_secondary  = 1
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         primary        = equip_forwardSrsLow()
         secondary      = equip_rangedSrs()
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Corvette
      if class == "Corvette" then
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsLow() )
         secondary      = equip_secondarySrs()
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

      -- Destroyer
      if class == "Destroyer" then
         scramble       = true
         primary        = icmb( equip_forwardSrsMed(), equip_turretSrsMed() )
         secondary      = equip_secondarySrs()
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         medium         = equip_mediumMed()
         low            = equip_lowMed()
         apu            = equip_apuMed()
      end

   else
      primary        = equip_turretSrsHig()
      secondary      = equip_secondarySrs()
      use_primary    = nhigh-2
      use_secondary  = 2
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end

   equip_ship( p, scramble, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low )
end
