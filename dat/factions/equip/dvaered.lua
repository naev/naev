-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does dvaered pilot equipping
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
   return { "Unicorp Mace Launcher" }
end
function equip_secondaryDva ()
   return { "Unicorp Mace Launcher" }
end



--[[
-- @brief Equips a dvaered military type ship.
--]]
function equip_dvaeredMilitary( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local use_forward, use_turrets, use_medturrets
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   weapons     = {}

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()
      cores = {
         {"Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Ultralight Combat Plating"},
         {"Tricon Zephyr II Engine", "Milspec Orion 3701 Core System", "S&K Light Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      -- Scout
      if class == "Scout" then
         equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Ultralight Combat Plating")
         use_forward    = rnd.rnd(1,#nhigh)
         addWeapons( equip_forwardLow(), use_forward )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
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

      -- Bomber
      elseif class == "Bomber" then
         use_forward    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_forward
         addWeapons( equip_forwardDvaLow(), use_forward )
         addWeapons( equip_secondaryDva(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      cores = {
         {"Tricon Cyclone Engine", "Milspec Orion 4801 Core System", "S&K Medium Combat Plating"},
         {"Tricon Cyclone II Engine", "Milspec Orion 5501 Core System", "S&K Medium-Heavy Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      use_secondary  = rnd.rnd(1,2)
      use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
      use_forward    = nhigh - use_secondary - use_turrets
      addWeapons( equip_secondaryDva(), use_secondary )
      addWeapons( equip_turretDvaMed(), use_turrets )
      addWeapons( equip_forwardDvaMed(), use_forward )
      medium         = equip_mediumMed()
      low            = equip_lowMed()

   else -- "large"
      -- TODO: Divide into carrier and cruiser classes.
      cores = {
         {"Tricon Typhoon Engine", "Milspec Orion 9901 Core System", "S&K Heavy Combat Plating"},
         {"Tricon Typhoon II Engine", "Milspec Orion 9901 Core System", "S&K Superheavy Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

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

      end
   equip_ship( p, false, weapons, medium, low, 
               use_medium, use_low )
end
