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

   -- Split by type
   if shiptype == "civilian" then
      equip_genericCivilian( p, shipsize )
   elseif shiptype == "merchant" then
      equip_genericMerchant( p, shipsize )
   elseif shiptype == "military" then
      equip_genericMilitary( p, shipsize )
   elseif shiptype == "robotic" then
      equip_genericRobotic( p, shipsize )
   end
end


--[[
-- @brief Equips a generic civilian type ship.
--]]
function equip_genericCivilian( p, shipsize )
   local medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   apu         = { }
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
   equip_ship( p, true, weapons, medium, low, apu,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic merchant type ship.
--]]
function equip_genericMerchant( p, shipsize )
   local medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   apu         = { }
   weapons     = {}
   use_primary = rnd.rnd(1,nhigh) -- Use fewer slots
   use_secondary = 0
   use_medium  = 0
   use_low     = 0

   -- Equip by size
   if shipsize == "small" then
      addWeapons( equip_forwardLow(), use_primary )
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
   equip_ship( p, true, weapons, medium, low, apu,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic military type ship.
--]]
function equip_genericMilitary( p, shipsize )
   local medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   apu         = { }

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
         apu            = equip_apuLow()

      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardLow(), use_primary )
         addWeapons( equip_rangedLow(), use_secondary )
         medium         = equip_mediumLow()
         low            = equip_lowLow()
         apu            = equip_apuLow()
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
      apu            = equip_apuMed()

   else
      use_primary    = nhigh-2
      use_secondary  = 2
      addWeapons( equip_turretMedLow(), use_primary )
      addWeapons( equip_secondaryMedLow(), use_secondary )
      medium         = equip_mediumHig()
      low            = equip_lowHig()
      apu            = equip_apuHig()
   end
   equip_ship( p, false, weapons, medium, low, apu,
               use_medium, use_low )
end


--[[
-- @brief Equips a generic robotic type ship.
--]]
function equip_genericRobotic( p, shipsize )
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end



