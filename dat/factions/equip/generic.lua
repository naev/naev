-- Outfit definitions
include("dat/factions/equip/outfits.lua")


-- Table of available core systems by class.
equip_classOutfits_coreSystems = {
   ["Yacht"] = {
      "Unicorp PT-100 Core System"
   },
   ["Luxury Yacht"] = {
      "Unicorp PT-100 Core System"
   },
   ["Scout"] = {
      "Unicorp PT-100 Core System", "Milspec Aegis 2201 Core System"
   },
   ["Courier"] = {
      "Unicorp PT-200 Core System", "Milspec Aegis 3601 Core System"
   },
   ["Freighter"] = {
      "Unicorp PT-600 Core System", "Milspec Aegis 5401 Core System"
   },
   ["Armoured Transport"] = {
      "Milspec Aegis 5401 Core System", "Milspec Orion 5501 Core System"
   },
   ["Fighter"] = {
      "Unicorp PT-200 Core System", "Milspec Orion 3701 Core System"
   },
   ["Bomber"] = {
      "Unicorp PT-200 Core System", "Milspec Orion 3701 Core System"
   },
   ["Corvette"] = {
      "Unicorp PT-500 Core System", "Milspec Orion 4801 Core System"
   },
   ["Destroyer"] = {
      "Unicorp PT-600 Core System", "Milspec Orion 5501 Core System"
   },
   ["Cruiser"] = {
      "Unicorp PT-1000 Core System", "Milspec Orion 9901 Core System"
   },
   ["Carrier"] = {
      "Milspec Orion 9901 Core System"
   },
   ["Drone"] = {
      "Milspec Orion 2301 Core System"
   },
   ["Heavy Drone"] = {
      "Milspec Orion 3701 Core System"
   }
}


-- Table of available engines by class.
equip_classOutfits_engines = {
   ["Yacht"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine"
   },
   ["Luxury Yacht"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine"
   },
   ["Scout"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Courier"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine", "Melendez Ox XL Engine"
   },
   ["Freighter"] = {
      "Unicorp Falcon 1200 Engine", "Malendez Buffalo XL Engine"
   },
   ["Armoured Transport"] = {
      "Malendez Buffalo XL Engine"
   },
   ["Fighter"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine"
   },
   ["Bomber"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine"
   },
   ["Corvette"] = {
      "Unicorp Falcon 550 Engine", "Nexus Arrow 550 Engine",
      "Tricon Cyclone Engine"
   },
   ["Destroyer"] = {
      "Unicorp Falcon 1200 Engine", "Nexus Arrow 1200 Engine",
      "Tricon Cyclone II Engine"
   },
   ["Cruiser"] = {
      "Unicorp Eagle 6500 Engine", "Nexus Bolt 6500 Engine",
      "Tricon Typhoon II Engine"
   },
   ["Carrier"] = {
      "Nexus Bolt 6500 Engine", "Tricon Typhoon II Engine",
      "Malendez Mammoth XL Engine"
   },
   ["Drone"] = {
      "Tricon Zephyr Engine"
   },
   ["Heavy Drone"] = {
      "Tricon Zephyr II Engine"
   }
}


-- Table of available hulls by class.
equip_classOutfits_hulls = {
   ["Yacht"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating"
   },
   ["Luxury Yacht"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating"
   },
   ["Scout"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating"
   },
   ["Courier"] = {
      "Unicorp D-4 Light Plating", "S&K Small Cargo Hull"
   },
   ["Freighter"] = {
      "Unicorp D-12 Medium Plating", "S&K Medium Cargo Hull"
   },
   ["Armoured Transport"] = {
      "S&K Medium Cargo Hull"
   },
   ["Fighter"] = {
      "Unicorp D-4 Light Plating", "Unicorp B-4 Light Plating",
      "S&K Light Stealth Plating", "S&K Light Combat Plating"
   },
   ["Bomber"] = {
      "Unicorp D-4 Light Plating", "Unicorp B-4 Light Plating",
      "S&K Light Stealth Plating", "S&K Light Combat Plating"
   },
   ["Corvette"] = {
      "Unicorp D-8 Medium Plating", "Unicorp B-8 Medium Plating",
      "S&K Medium Stealth Plating", "S&K Medium Combat Plating"
   },
   ["Destroyer"] = {
      "Unicorp D-12 Medium Plating", "Unicorp B-12 Medium Plating",
      "S&K Medium-Heavy Stealth Plating", "S&K Medium-Heavy Combat Plating"
   },
   ["Cruiser"] = {
      "Unicorp D-20 Heavy Plating", "Unicorp B-20 Heavy Plating",
      "S&K Superheavy Combat Plating"
   },
   ["Carrier"] = {
      "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
   },
   ["Drone"] = {
      "S&K Ultralight Stealth Plating"
   },
   ["Heavy Drone"] = {
      "S&K Light Stealth Plating"
   }
}


-- Tables of available weapons by class.
-- Each table is split up into sub-tables that are iterated
-- through when equipping a ship. These tables include a "num" field which
-- indicates how many of the chosen weapon to equip before moving on to the
-- next set; if nil, the chosen weapon will be equipped as many times as
-- possible. For example, if you list 3 tables with "num" set to 2, 1, and nil
-- respectively, two of a weapon from the first table will be equipped,
-- followed by one of a weapon from the second table, and then finally all
-- remaining slots will be filled with a weapon from the third table.
-- In general, the final table should be a table of possible primary weapons,
-- and in front should be a table of secondary weapons restricted to 1 or 2
-- (depending on the type of ship).
--
-- If, rather than equipping multiples of the same outfit you would like to
-- select a random outfit `num` times, you can do so by setting "varied" to
-- true.
equip_classOutfits_weapons = {
   ["Yacht"] = {
      {
         "Laser Cannon MK1", "Laser Cannon MK2", "Razor MK1", "Razor MK2",
         "Laser PD MK1", "Turreted Gauss Gun"
      }
   },
   ["Luxury Yacht"] = {
      {
         "Laser Cannon MK1", "Laser Cannon MK2", "Razor MK1", "Razor MK2"
      }
   },
   ["Scout"] = {
      {
         "Laser PD MK1", "Laser PD MK2", "Razor Turret MK1",
         "Turreted Gauss Gun"
      }
   },
   ["Courier"] = {
      {
         "Laser PD MK1", "Laser PD MK2", "Razor Turret MK1",
         "Turreted Gauss Gun"
      }
   },
   ["Freighter"] = {
      {
         num = 1;
         "Laser Turret MK1", "Laser Turret MK2", "EMP Grenade Launcher",
         "Pulse Beam", "Enygma Systems Turreted Fury Launcher"
      },
      {
         "Laser Turret MK1", "Laser Turret MK2"
      }
   },
   ["Armoured Transport"] = {
      {
         num = 1;
         "Pulse Beam", "Enygma Systems Turreted Fury Launcher",
         "Heavy Laser", "Heavy Ripper Turret", "Lancelot Fighter Bay"
      },
      {
         "Laser Turret MK2", "Laser Turret MK3"
      }
   },
   ["Fighter"] = {
      {
         num = 1;
         "Mass Driver MK1", "Ion Cannon", "Unicorp Mace Launcher",
         "Unicorp Banshee Launcher", "Orion Lance", "Shattershield Lance",
         "Unicorp Headhunter Launcher", "Unicorp Fury Launcher",
         "Unicorp Medusa Launcher"
      },
      {
         "Plasma Blaster MK1", "Plasma Blaster MK2", "Gauss Gun",
         "Vulcan Gun", "Ripper Cannon"
      }
   },
   ["Bomber"] = {
      {
         num = 3;
         "TeraCom Fury Launcher", "TeraCom Medusa Launcher",
         "Unicorp Headhunter Launcher", "Unicorp Mace Launcher",
         "Unicorp Banshee Launcher"
      },
      {
         "Gauss Gun", "Vulcan Gun", "Laser Cannon MK2", "Plasma Blaster MK2"
      }
   },
   ["Corvette"] = {
      {
         num = 1;
         "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
         "TeraCom Medusa Launcher"
      },
      {
         num = 2;
         "Mass Driver MK1", "Mass Driver MK2", "Heavy Ion Cannon",
         "Laser Turret MK1", "Plasma Turret MK2", "Razor Turret MK2"
      },
      {
         "Ripper Cannon", "Plasma Blaster MK2", "Laser Cannon MK2",
         "Vulcan Gun", "Ion Cannon"
      }
   },
   ["Destroyer"] = {
      {
         num = 2;
         "Railgun", "Heavy Ripper Turret", "Heavy Laser", "Orion Beam",
         "Grave Beam", "Laser Turret MK3", "Razor Turret MK2"
      },
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher", "Unicorp Caesar IV Launcher",
         "Unicorp Headhunter Launcher", "TeraCom Medusa Launcher"
      },
      {
         "Laser Turret MK2", "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Cruiser"] = {
      {
         num = 2;
         "Turbolaser", "Ragnarok Beam", "Grave Beam", "Railgun Turret"
      },
      {
         "Heavy Laser", "Heavy Ripper Turret", "Railgun Turret"
      },
      {
         "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Carrier"] = {
      {
         num = 2;
         "Turbolaser", "Ragnarok Beam"
      },
      {
         num = 2;
         "Heavy Laser", "Grave Beam", "Railgun Turret"
      },
      {
         "Lancelot Fighter Bay"
      },
      {
         "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Drone"] = {
      {
         "Neutron Disruptor"
      }
   },
   ["Heavy Drone"] = {
      {
         "Heavy Neutron Disruptor"
      },
      {
         "Electron Burst Cannon"
      }
   }
}


-- Tables of available utilities by class.
-- Each table is split up into sub-tables that are iterated
-- through when equipping a ship. These tables include a "num" field which
-- indicates how many of the chosen weapon to equip before moving on to the
-- next set; if nil, the chosen weapon will be equipped as many times as
-- possible. For example, if you list 3 tables with "num" set to 2, 1, and nil
-- respectively, two of a weapon from the first table will be equipped,
-- followed by one of a weapon from the second table, and then finally all
-- remaining slots will be filled with a weapon from the third table.
-- In general, the final table should be a table of possible primary weapons,
-- and in front should be a table of secondary weapons restricted to 1 or 2
-- (depending on the type of ship).
--
-- If, rather than equipping multiples of the same outfit you would like to
-- select a random outfit `num` times, you can do so by setting "varied" to
-- true.
equip_classOutfits_utilities = {
   ["Yacht"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Jump Scanner",
         "Generic Afterburner"
      }
   },
   ["Luxury Yacht"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Small Shield Booster"
      }
   },
   ["Scout"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Small Shield Booster", "Jump Scanner",
         "Generic Afterburner", "Emergency Shield Booster"
      }
   },
   ["Courier"] = {
      {
         varied = true;
         "Reactor Class I", "Small Shield Booster", "Unicorp Scrambler",
         "Hellburner", "Emergency Shield Booster"
      }
   },
   ["Freighter"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MKI"
      }
   },
   ["Armoured Transport"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MKI"
      }
   },
   ["Fighter"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Emergency Shield Booster",
         "Reverse Thrusters"
      }
   },
   ["Bomber"] = {
      {
         varied = true;
         "Reactor Class I", "Milspec Scrambler", "Small Shield Booster",
         "Emergency Shield Booster", "Hellburner", "Reverse Thrusters"
      }
   },
   ["Corvette"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MKI", "Hellburner"
      }
   },
   ["Destroyer"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Droid Repair Crew",
         "Boarding Androids MKI"
      }
   },
   ["Cruiser"] = {
      {
         varied = true;
         "Reactor Class III", "Large Shield Booster", "Droid Repair Crew",
         "Boarding Androids MKII"
      }
   },
   ["Carrier"] = {
      {
         varied = true;
         "Reactor Class III", "Large Shield Booster", "Droid Repair Crew",
         "Boarding Androids MKII"
      }
   },
   ["Drone"] = {
      {
         "Reactor Class I"
      }
   },
   ["Heavy Drone"] = {
      {
         num = 1
         "Unicorp Scrambler"
      },
      {
         "Reactor Class I"
      }
   }
}


-- Helper functions
include("dat/factions/equip/helper.lua")

--[[
-- @brief Does generic pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_generic( p )
   -- Start with an empty ship
   p:rmOutfit("all")
   p:rmOutfit("cores")

   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )
   
   -- Equip core outfits. This process is separate from the other outfits, because cores were introduced
   -- later, and the outfitting routine should be fairly granular and tweakable.
   --equip_cores(p)

   -- Split by type
   if shiptype == "civilian" and p:faction() ~= faction.get("Trader") then
      if shipsize == "small" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Unicorp Hawk 150 Engine", "Unicorp PT-100 Core System", "Unicorp D-2 Light Plating"},
            {"Unicorp Hawk 300 Engine", "Unicorp PT-200 Core System", "Unicorp D-4 Light Plating"}
         }))
      elseif shipsize == "medium" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Unicorp Falcon 550 Engine", "Unicorp PT-500 Core System", "Unicorp D-8 Medium Plating"},
            {"Unicorp Falcon 1200 Engine", "Unicorp PT-600 Core System", "Unicorp D-12 Medium Plating"}
         }))
      else
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Unicorp Eagle 4500 Engine", "Unicorp PT-1000 Core System", "Unicorp D-16 Heavy Plating"},
            {"Unicorp Eagle 6500 Engine", "Unicorp PT-1000 Core System", "Unicorp D-20 Heavy Plating"}
         }))
      end
      equip_genericCivilian( p, shipsize )
   elseif shiptype == "merchant" or p:faction() == faction.get("Trader") then
      if shipsize == "small" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Melendez Ox Engine", "Unicorp PT-100 Core System", "S&K Small Cargo Hull"},
            {"Melendez Ox XL Engine", "Unicorp PT-200 Core System", "S&K Small Cargo Hull"}
         }))
      elseif shipsize == "medium" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Melendez Buffalo Engine", "Unicorp PT-500 Core System", "S&K Medium Cargo Hull"},
            {"Melendez Buffalo XL Engine", "Unicorp PT-500 Core System", "S&K Medium Cargo Hull"}
         }))
      else
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Melendez Mammoth Engine", "Unicorp PT-1000 Core System", "S&K Large Cargo Hull"},
            {"Melendez Mammoth XL Engine", "Unicorp PT-1000 Core System", "S&K Large Cargo Hull"}
         }))
      end
      equip_genericMerchant( p, shipsize )
   elseif shiptype == "military" then
      if shipsize == "small" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Dart 150 Engine", "Milspec Hermes 2202 Core System", "Unicorp B-2 Light Plating"},
            {"Nexus Dart 300 Engine", "Milspec Hermes 3602 Core System", "Unicorp B-4 Light Plating"}
         }))
      elseif shipsize == "medium" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Arrow 550 Engine", "Milspec Hermes 4702 Core System", "Unicorp B-8 Medium Plating"},
            {"Nexus Arrow 1200 Engine", "Milspec Hermes 5402 Core System", "Unicorp B-12 Medium Plating"}
         }))
      else
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Bolt 4500 Engine", "Milspec Hermes 9802 Core System", "Unicorp B-16 Heavy Plating"},
            {"Nexus Bolt 6500 Engine", "Milspec Hermes 9802 Core System", "Unicorp B-20 Heavy Plating"}
         }))
      end
      equip_genericMilitary( p, shipsize )
   elseif shiptype == "robotic" then
      if shipsize == "small" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Dart 150 Engine", "Milspec Hermes 2202 Core System", "Unicorp B-2 Light Plating"},
            {"Nexus Dart 300 Engine", "Milspec Hermes 3602 Core System", "Unicorp B-4 Light Plating"}
         }))
      elseif shipsize == "medium" then
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Arrow 550 Engine", "Milspec Hermes 4702 Core System", "Unicorp B-8 Medium Plating"},
            {"Nexus Arrow 1200 Engine", "Milspec Hermes 5402 Core System", "Unicorp B-12 Medium Plating"}
         }))
      else
         equip_cores(p, equip_getCores(p, shipsize, {
            {"Nexus Bolt 4500 Engine", "Milspec Hermes 9802 Core System", "Unicorp B-16 Heavy Plating"},
            {"Nexus Bolt 6500 Engine", "Milspec Hermes 9802 Core System", "Unicorp B-20 Heavy Plating"}
         }))
      end

      equip_genericRobotic( p, shipsize )
   end
end

--[[
-- @brief Equips a ship with core outfits.
--
--    @param p Pilot to equip
--]]
function equip_cores( p, engine, system, hull )
   p:rmOutfit("cores")
   if p:addOutfit(system, 1, false) == 0 then warn("Could not equip " .. system .. " on pilot " .. p:name() .. "!") end
   if p:addOutfit(hull,   1, false) == 0 then warn("Could not equip " .. hull .. " on pilot " .. p:name() .. "!") end
   if p:addOutfit(engine, 1, false) == 0 then warn("Could not equip " .. engine .. " on pilot " .. p:name() .. "!") end
end

--[[
-- @brief Gets suitably-sized core outfits for a ship.
--
--    @param p Pilot to get cores for
--    @param shipsize Size of the ship, including its core slots
--    @param cores Table of tables, containing 2 size classes with 3 cores each
--]]
function equip_getCores( p, shipsize, cores )
   local mass = p:stats()['mass']

   -- These thresholds are somewhat arbitrary, but serve to delineate between
   -- large and small ships within a class, e.g. a Shark (light fighter) is
   -- 45t, while a Lancelot (heavy fighter) is 80t. Thus, the Shark is under
   -- the small slot's threshold, while the Lancelot is over.
   if shipsize == "small" then
       threshold = 60
   elseif shipsize == "medium" then
      threshold = 220
   else
      threshold = 1800
   end

   if p:stats()['mass'] <= threshold then
      return unpack(cores[1])
   end

   return unpack(cores[2])
end

--[[
-- @brief Equips a generic civilian type ship.
--]]
function equip_genericCivilian( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }

   weapons = {}
   use_primary = rnd.rnd(nhigh) -- Use fewer slots
   use_secondary = 0
   use_medium  = 0
   use_low     = 0

   -- Per ship type
   if shipsize == "small" then
      addWeapons( equip_forwardLow(), use_primary )
      medium   = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.8 then
         use_medium = 1
      end
   else
      addWeapons( equip_turretLow(), use_primary )
      medium   = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.5 then
         use_medium = 1
      end
      low      = { "Plasteel Plating" }
      if rnd.rnd() > 0.5 then
         use_low = 1
      end
   end
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic merchant type ship.
--]]
function equip_genericMerchant( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }

   weapons     = {}
   use_primary = rnd.rnd(1,nhigh) -- Use fewer slots
   use_secondary = 0
   use_medium  = 0
   use_low     = 0

   -- Equip by size
   if shipsize == "small" then
      r = rnd.rnd()
      if r > 0.9 or r > 0.2 and use_primary < 2 then -- 10% chance of all-turrets.
         addWeapons( equip_turretLow(), use_primary )
      elseif r > 0.2 then -- 70% chance of mixed loadout.
         use_turrets = rnd.rnd( 1, use_primary-1 )
         use_primary = use_primary - use_turrets
         addWeapons( equip_turretLow(), use_turrets )
         addWeapons( equip_forwardLow(), use_primary )
      else -- Poor guy gets no turrets.
         addWeapons( equip_forwardLow(), use_primary )
      end

      medium   = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.8 then
         use_medium = 1
      end
   elseif shipsize == "medium" then
      addWeapons( equip_turretLow(), use_primary )

      use_secondary = math.min(1, nhigh - use_primary)
      if use_secondary > 0 then
         addWeapons( { "EMP Grenade Launcher" }, use_secondary )
      end

      medium   = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.6 then
         use_medium = 1
      end
      low    = { "Plasteel Plating" }
      if rnd.rnd() > 0.6 then
         use_low = 1
      end
   else
      addWeapons( equip_turretLow(), use_primary )

      use_secondary = math.min(2, nhigh - use_primary)
      if use_secondary > 0 then
         addWeapons( { "EMP Grenade Launcher" }, use_secondary )
      end

      medium = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.4 then
         use_medium = 1
      end
      low    = { "Plasteel Plating" }
      if rnd.rnd() > 0.6 then
         use_low = 1
      end
   end
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic military type ship.
--]]
function equip_genericMilitary( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   weapons     = {}

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Scout
      if class == "Scout" then
         use_primary    = rnd.rnd(1,nhigh)
         addWeapons( equip_forwardLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Fighter
      elseif class == "Fighter" then
         use_primary    = nhigh-1
         use_secondary  = 1
         addWeapons( equip_forwardLow(), use_primary )
         addWeapons( equip_secondaryLow(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()


      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardLow(), use_primary )
         addWeapons( equip_rangedLow(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()

      end

   elseif shipsize == "medium" then
      use_secondary  = rnd.rnd(1,2)
      use_primary    = nhigh - use_secondary
      if rnd.rnd() < 0.6 then
         addWeapons( equip_forwardMedLow(), use_primary )
      else
         addWeapons( equip_turretLow(), use_primary )
      end
      addWeapons( equip_secondaryMedLow(), use_secondary )
      medium         = equip_mediumMed()
      low            = equip_lowMed()


   else
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( equip_turretHigMedLow(), use_primary )
      addWeapons( equip_secondaryMedLow(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()

   end
   equip_ship( p, false, weapons, medium, low,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic robotic type ship.
--]]
function equip_genericRobotic( p, shipsize )
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end



