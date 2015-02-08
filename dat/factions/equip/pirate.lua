-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Start with an empty ship
   p:rmOutfit("all")
   p:rmOutfit("cores")

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
   return { "Laser Turret MK1", "Razor Turret MK1", "Turreted Gauss Gun" }
end
function equip_turretPirMed ()
   return { "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun" }
end
function equip_turretPirHig ()
   return { "Railgun Turret", "Heavy Laser" }
end
function equip_secondaryPirLow ()
   return { "Unicorp Mace Launcher" }
end
function equip_secondaryPirMedLow ()
   return { "Unicorp Fury Launcher", "Unicorp Banshee Launcher" }
end


--[[
-- @brief Equips a pirate military type ship.
--]]
function equip_pirateMilitary( p, shipsize )
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }

   weapons     = {}

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Smaller pirate vessels should have a fair amount of variance in their
      -- core outfit quality.
      small_engines = { "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine" }
      large_engines = { "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine" }
      small_hulls   = { "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
            "S&K Ultralight Combat Plating" }
      large_hulls   = { "Unicorp D-4 Light Plating", "Unicorp B-4 Light Plating",
            "S&K Light Combat Plating" }

      local small_engine = small_engines[ rnd.rnd(1, #small_engines) ]
      local large_engine = large_engines[ rnd.rnd(1, #large_engines) ]
      local small_hull   = small_hulls[ rnd.rnd(1, #small_hulls) ]
      local large_hull   = large_hulls[ rnd.rnd(1, #large_hulls) ]

      cores = {
         { small_engine, "Milspec Prometheus 2203 Core System", small_hull },
         { large_engine, "Milspec Prometheus 3603 Core System", large_hull }
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      -- Scout - shouldn't exist
      if class == "Scout" then
         equip_cores(p, small_engine, "Milspec Prometheus 2203 Core System", small_hull)
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


      -- Bomber
      elseif class == "Bomber" then
         equip_cores(p, large_engine, "Milspec Aegis 3601 Core System", large_hull)
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardPirLow(), use_primary )
         addWeapons( equip_secondaryPirLow(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()

      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      cores = {
         {"Nexus Arrow 550 Engine", "Milspec Prometheus 4703 Core System", "S&K Medium Combat Plating"},
         {"Nexus Arrow 1200 Engine", "Milspec Prometheus 5403 Core System", "Unicorp B-12 Medium Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

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


   else
      equip_cores(p, "Nexus Bolt 4500 Engine", "Milspec Prometheus 9803 Core System", "S&K Heavy Combat Plating")
      primary        = icmb( equip_turretPirHig(), equip_turretPirMed() )
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( primary, use_primary )
      addWeapons( equip_secondaryPirMedLow(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()

   end
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low )
end
