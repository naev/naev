--[[
-- @brief Does dvaered pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_dvaered( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "military" then
      equip_dvaeredMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


function equip_forwardDvaLow ()
   return { "Vulcan Gun", "Shredder" }
end
function equip_forwardDvaMed ()
   return { "Shredder", "Mass Driver MK2", "Mass Driver MK3" }
end
function equip_forwardDvaHig ()
   return { "Railgun", "Repeating Railgun" }
end
function equip_turretDvaLow ()
   return { "Turreted Gauss Gun" }
end
function equip_turretDvaMed ()
   return { "Turreted Vulcan Gun" }
end
function equip_turretDvaHig ()
   return { "Railgun Turret" }
end
function equip_rangedDva ()
   return { "Mace Launcher" }
end
function equip_secondaryDva ()
   return { "Mace Launcher" }
end



--[[
-- @brief Equips a dvaered military type ship.
--]]
function equip_dvaeredMilitary( p, shipsize )
   local medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local use_forward, use_turrets, use_medturrets
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Civilian Jammer" }
   apu         = { }
   weapons     = {}

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout
      if class == "Scout" then
         use_forward    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardLow(), use_forward )
         medium         = { "Generic Afterburner", "Milspec Jammer" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         use_secondary  = 1
         use_forward    = nhigh - use_secondary
         addWeapons( equip_forwardDvaLow(), use_forward )
         addWeapons( equip_secondaryDva(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         use_forward    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_forward
         addWeapons( equip_forwardDvaLow(), use_forward )
         addWeapons( equip_secondaryDva(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
      end

   elseif shipsize == "medium" then
      use_secondary  = rnd.rnd(1,2)
      use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
      use_forward    = nhigh - use_secondary - use_turrets
      addWeapons( equip_secondaryDva(), use_secondary )
      addWeapons( equip_turretDvaMed(), use_turrets )
      addWeapons( equip_forwardDvaMed(), use_forward )
      medium         = equip_mediumMed()
      low            = equip_lowMed()
      apu            = equip_apuMed()

   else -- "large"
      use_secondary  = 2
      use_turrets = nhigh - use_secondary - rnd.rnd(2,3)
      if rnd.rnd() > 0.4 then -- Anti-fighter variant.
         use_medturrets = nhigh - use_secondary - use_turrets
         addWeapons( equip_turretDvaMed(), use_medturrets )
      else -- Forward variant.
         use_forward    = nhigh - use_secondary - use_turrets
         addWeapons( equip_forwardDvaHig(), use_forward )
      end
      addWeapons( equip_secondaryDva(), use_secondary )
      addWeapons( equip_turretDvaHig(), use_turrets )
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end
   equip_ship( p, false, weapons, medium, low, apu,
               use_medium, use_low )
end
