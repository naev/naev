include("scripts/pilot/generic.lua")

--[[
include("ai/equip/empire.lua")
include("ai/equip/dvaerd.lua")
include("ai/equip/trader.lua")
include("ai/equip/pirate.lua")
--]]


--[[
-- @brief Equips a pilot
--
--    @param p Pilot to equip
--    @param f Faction to which pilot belongs
--]]
function equip ( p, f )
   --[[
   if f == faction.get( "Empire" ) then
      equip_empire( p )
   elseif f == faction.get( "Dvaered" ) then
      equip_dvaered( p )
   elseif f == faction.get( "Trader" ) then
      equip_trader( p )
   elseif f == faction.get( "Pirate" ) then
      equip_pirate( p )
   else
      equip_generic( p )
   end
   --]]
   equip_generic( p )
end


--[[
-- @brief Handles the ship class by splitting it up into type/size.
--
-- Valid types are:
--  - civilian
--  - merchant
--  - military
--  - robotic
--
-- Valid sizes are:
--  - small
--  - medium
--  - large
--
--    @return Two parameters, first would be type, second would be size.
--]]
function equip_getShipBroad( p )
   local class = p:shipClass()

   -- Civilian
   if class == "Yacht" or class == "Luxury Yacht" then
      return "civilian", "small"
   elseif class == "Cruise Ship" then
      return "civilian", "medium"

   -- Merchant
   elseif class == "Courier" then
      return "merchant", "small"
   elseif class == "Freighter" then
      return "merchant", "medium"
   elseif class == "Bulk Carrier" then
      return "merchant", "large"

   -- Military
   elseif class == "Scout" or class == "Fighter" or class == "Bomber"  then
      return "military", "small"
   elseif class == "Corvette" or class == "Destroyer" then
      return "military", "medium"
   elseif class == "Cruiser" or class == "Carrier" then
      return "military", "large"

   -- Robotic
   elseif class == "Drone" then
      return "robotic", "small"
   elseif class == "Heavy Drone" then
      return "robotic", "medium"
   elseif class == "Mothership" then
      return "robotic", "large"

   -- Unknown
   else
      print("Unknown ship of class '" .. class .. "'")
   end
end


--[[
-- @brief Fills the pilot slots with the outfits in the arrays.
--
--    @param p Pilot to fill outfit slots.
--    @param high High slots to use.
--    @param medium Medium slots to use.
--    @param low Low slots to use.
--    @param use_high Number of high slots to use (default max).
--    @param use_medium Number of medium slots to use (default max).
--    @param use_low Number of low slots to use (default max).
--]]
function equip_fillSlots( p, high, medium, low, use_high, use_medium, use_low )
   local nhigh, nmedium, nlow = p:shipSlots()
   -- Defaults
   use_high    = use_high or nhigh
   use_medium  = use_medium or nmedium
   use_low     = use_low or nlow
   -- Medium slots - for cpu/energy regen limits
   if #medium > 0 then
      local i = 0
      while i < use_medium do
         p:addOutfit( medium[ rnd.rnd(1,#medium) ] )
         i = i + 1
      end
   end
   -- Low slots
   if #low > 0 then
      local i = 0
      while i < use_low do
         p:addOutfit( low[ rnd.rnd(1,#low) ] )
         i = i + 1
      end
   end
   -- High slots
   if #high > 0 then
      local i = 0
      while i < use_high do
         p:addOutfit( high[ rnd.rnd(1,#high) ] )
         i = i + 1
      end
   end
end


--[[
-- @brief Does generic pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_generic( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p )

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
   local nhigh, nmedium, nlow = p:shipSlots()
   local high, medium, low
   local use_high, use_medium, use_low
   use_high = rnd.rnd(nhigh) -- Use fewer slots
   if shipsize == "small" then
      high   = { "Laser Cannon", "Plasma Blaster" }
      medium = { "Civilian Jammer" }
      if rnd.rnd() > 0.8 then
         use_medium = 1
      else
         use_medium = 0
      end
      low    = { }
   else
      high   = { "Laser Turret" }
      medium = { "Civilian Jammer" }
      if rnd.rnd() > 0.5 then
         use_medium = 1
      else
         use_medium = 0
      end
      low    = { }
   end
   equip_fillSlots( p, high,     medium,     low,
                       use_high, use_medium, use_low)
end


--[[
-- @brief Equips a generic merchant type ship.
--]]
function equip_genericMerchant( p, shipsize )
   local nhigh, nmedium, nlow = p:shipSlots()
   local high, medium, low
   local use_high, use_medium, use_low
   use_high = rnd.rnd(1,nhigh) -- Use fewer slots
   if shipsize == "small" then
      high   = { "Laser Cannon", "Plasma Blaster" }
      medium = { "Civilian Jammer" }
      if rnd.rnd() > 0.8 then
         use_medium = 1
      else
         use_medium = 0
      end
      low    = { }
   elseif shipsize == "medium" then
      high   = { "Laser Turret" }
      medium = { "Civilian Jammer" }
      if rnd.rnd() > 0.6 then
         use_medium = 1
      else
         use_medium = 0
      end
      low    = { }
   else
      high   = { "Laser Turret", "EMP Grenade Launcher" }
      medium = { "Civilian Jammer" }
      if rnd.rnd() > 0.4 then
         use_medium = 1
      else
         use_medium = 0
      end
      low    = { }
   end
   equip_fillSlots( p, high,     medium,     low,
                       use_high, use_medium, use_low)
end


--[[
-- @brief Equips a generic military type ship.
--]]
function equip_genericMilitary( p, shipsize )
   local nhigh, nmedium, nlow = p:shipSlots()
   local high, medium, low
   local use_high, use_medium, use_low
   if shipsize == "small" then
      local class = p:shipClass()
      if class == "Scout" then
         high   = { "Laser Cannon" }
         medium = { "Reactor Class I", "Generic Afterburner", "Milspec Jammer" }
         low    = { "Solar Panel", }
      elseif class == "Fighter" then
         high   = { "Laser Cannon", "Plasma Blaster", "40mm Autocannon" }
         p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-1 ) -- Adds uniformity
         p:addOutfit( "Seeker Launcher" )
         high   = { }
         medium = { "Reactor Class I", "Generic Afterburner", "Milspec Jammer",
                    "Auxiliary Processing Unit I" }
         low    = { "Shield Capacitor", "Plasteel Plating", "Engine Reroute", }
      elseif class == "Bomber" then
         high   = { "Laser Cannon", "Plasma Blaster" }
         p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-2 ) -- Adds uniformity
         p:addOutfit( "Seeker Launcher", 2 )
         high   = { }
         medium = { "Reactor Class I", "Generic Afterburner", "Milspec Jammer",
                    "Auxiliary Processing Unit I" }
         low    = { "Shield Capacitor", "Plasteel Plating", "Engine Reroute", }
      end
   elseif shipsize == "medium" then
      high   = { "Laser Turret" }
      medium = { }
      low    = { }
   else
      high   = { "Laser Turret", "EMP Grenade Launcher" }
      medium = { }
      low    = { }
   end
   equip_fillSlots( p, high,     medium,     low,
                       use_high, use_medium, use_low)
end


--[[
-- @brief Equips a generic robotic type ship.
--]]
function equip_genericRobotic( p, shipsize )
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end



