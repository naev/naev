-- Outfit definitions
include("dat/factions/equip/outfits.lua")

-- Helper functions
include("dat/factions/equip/helper.lua")

--[[
-- @brief Does generic pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_generic( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )
   
   -- Equip core outfits. This process is separate from the other outfits, because cores were introduced
   -- later, and the outfitting routine should be fairly granular and tweakable.
   --equip_cores(p)

   -- Split by type
   if shiptype == "civilian" and p:faction() ~= faction.get("Trader") then
      if shipsize == "small" then      equip_cores(p, "Unicorp KRD-B12 Engine", "Unicorp PT-200 Core System", "Unicorp C-2 Light Plating")
      elseif shipsize == "medium" then equip_cores(p, "Unicorp KRD-G15 Engine", "Unicorp PT-500 Core System", "Unicorp D-8 Medium Plating")
      else                             equip_cores(p, "Unicorp KRD-T20 Engine", "Unicorp PT-1200 Core System", "Unicorp C-16 Heavy Plating")
      end
      equip_genericCivilian( p, shipsize )
   elseif shiptype == "merchant" or p:faction() == faction.get("Trader") then
      if shipsize == "small" then      equip_cores(p, "Unicorp AUR-50A Engine", "Unicorp PT-200 Core System", "Unicorp Small Cargo Hull")
      elseif shipsize == "medium" then equip_cores(p, "Unicorp JNS-54K Engine", "Unicorp PT-500 Core System", "Unicorp Medium Cargo Hull")
      else                             equip_cores(p, "Unicorp JNS-99K Engine", "Unicorp PT-1200 Core System", "Unicorp Large Cargo Hull")
      end
      equip_genericMerchant( p, shipsize )
   elseif shiptype == "military" then
      if shipsize == "small" then      equip_cores(p, "Tricon Naga Mk5 Engine", "Milspec Orion 2302 Core System", "Schafer & Kane Light Combat Plating")
      elseif shipsize == "medium" then equip_cores(p, "Tricon Centaur Mk3 Engine", "Milspec Orion 3702 Core System", "Schafer & Kane Medium Combat Plating Beta")
      else                             equip_cores(p, "Tricon Harpy Mk6 Engine", "Milspec Orion 4802 Core System", "Schafer & Kane Heavy Combat Plating Beta")
      end
      equip_genericMilitary( p, shipsize )
   elseif shiptype == "robotic" then
      if shipsize == "small" then      equip_cores(p, "Tricon Naga Mk5 Engine", "Milspec Orion 2302 Core System", "Schafer & Kane Light Combat Plating")
      elseif shipsize == "medium" then equip_cores(p, "Tricon Centaur Mk3 Engine", "Milspec Orion 3702 Core System", "Schafer & Kane Medium Combat Plating Beta")
      else                             equip_cores(p, "Tricon Harpy Mk6 Engine", "Milspec Orion 4802 Core System", "Schafer & Kane Heavy Combat Plating Beta")
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
      use_secondary = 1
      addWeapons( equip_turretLow(), use_primary )
      addWeapons( { "EMP Grenade Launcher" }, use_secondary )
      medium   = { "Unicorp Scrambler" }
      if rnd.rnd() > 0.6 then
         use_medium = 1
      end
      low    = { "Plasteel Plating" }
      if rnd.rnd() > 0.6 then
         use_low = 1
      end
   else
      use_secondary = 2
      addWeapons( equip_turretLow(), use_primary )
      addWeapons( { "EMP Grenade Launcher" }, use_secondary )
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
      addWeapons( equip_turretMedLow(), use_primary )
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



