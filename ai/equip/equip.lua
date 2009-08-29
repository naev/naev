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
-- Weapon definitions
--]]
function table_merge( t, ... )
   args = {...}
   for ak, av in ipairs( args ) do
      for k,v in ipairs(av) do
         t[ #t+1 ] = v
      end
   end
   return t
end
--[[
-- Forward mounts
--]]
function equip_forwardLow ()
   return { "Laser Cannon", "Plasma Blaster", "40mm Autocannon" }
end
function equip_forwardMed ()
   return { "Laser Cannon MK2", "Plasma Blaster MK2", "Ion Cannon" }
end
function equip_forwardHig ()
   return { "150mm Railgun", "Ripper MK2" }
end
function equip_forwardMedLow ()
   return table_merge( equip_forwardLow(), equip_forwardMed() )
end
function equip_forwardHigMedLow ()
   return table_merge( equip_forwardLow(), equip_forwardMed(), equip_forwardHig() )
end
--[[
-- Turret mounts
--]]
function equip_turretLow ()
   return { "Laser Turret" }
end
function equip_turretMed ()
   return { "Laser Turret MK2", "Heavy Ion Turret" }
end
function equip_turretHig ()
   return { "150mm Railgun Turret" }
end
function equip_turretMedLow ()
   return table_merge( equip_turretLow(), equip_turretMed() )
end
function equip_turretHigMedLow ()
   return table_merge( equip_turretLow(), equip_turretMed(), equip_turretHig() )
end
--[[
-- Secondary weapons
--]]
function equip_secondaryLow ()
   return { "Seeker Launcher", "Mace Launcher" }
end
function equip_secondaryMed ()
   return { "Banshee Launcher", "Headhunter Launcher" }
end
function equip_secondaryHig ()
   return { }
end
function equip_secondaryMedLow ()
   return table_merge( equip_secondaryLow(), equip_secondaryMed() )
end
function equip_secondaryHigMedLow ()
   return table_merge( equip_secondaryLow(), equip_secondaryMed(), equip_secondaryHig() )
end


--[[
-- @brief Ultimate equipment function, will set up a ship based on many parameters
--
--    @param p Pilot to equip.
--    @param scramble Use crazy assortment of primary/secondary weapons.
--    @param primary List of primary weapons to use.
--    @param secondary List of secondary weapons to use.
--    @param medium List of medium outfits to use.
--    @param low List of low outfits to use.
--    @param apu List of APU to use.
--    @param reactor List of reactors to use.
--    @param use_primary Amount of slots to use for primary (default nhigh-1).
--    @param use_secondary Amount of slots to use for secondary (default 1).
--    @param use_medium Amount of slots to use for medium outfits (default nmedium).
--    @param use_low Amount of slots to use for low outfits (default nlow).
--]]
function equip_ship( p, scramble, primary, secondary, medium, low, apu,
   use_primary, use_secondary, use_medium, use_low )

   --[[
   --    Variables
   --]]
   local nhigh, nmedium, nlow = p:shipSlots()
   local shipcpu = p:shipCPU()
   local shiptype, shipsize = equip_getShipBroad( p )
   local outfits = { }
   local i


   --[[
   --    Set up parameters that might be empty
   --]]
   use_primary    = use_primary or nhigh-1
   use_secondary  = use_secondary or 1
   use_medium     = use_medium or nmedium
   use_low        = use_low or nlow


   --[[
   --    Set up weapons
   --]]
   -- Check uniformity
   local po, so
   if scramble then
      po = primary[ rnd.rnd(1,#primary) ]
      so = secondary[ rnd.rnd(1,#secondary) ]
   end
   -- Primary
   i = 0
   local o = primary[ rnd.rnd(1,#primary) ]
   while i < use_primary do
      outfits[ #outfits+1 ] = po or primary[ rnd.rnd(1,#primary) ]
      i = i + 1
   end
   -- Secondary
   i = 0
   o = secondary[ rnd.rnd(1,#secondary) ]
   while i < use_secondary do
      outfits[ #outfits+1 ] = so or secondary[ rnd.rnd(1,#secondary) ]
      i = i + 1
   end
   -- Check CPU if we can add APU
   if apu ~= nil and #apu > 0 then
      local cpu_usage = 0
      for k,v in ipairs( outfits ) do
         cpu_usage = cpu_usage + p.outfitCPU( v )
      end
      local added = true
      while added and cpu_usage > shipcpu do -- Need to add APU
         added       = p:addOutfit( apu[ rnd.rnd(1,#apu) ] )
         shipcpu     = p:shipCPU()
         use_medium  = use_medium - 1 -- Discount from available medium slots
      end
   end
   -- Add high slots
   for k,v in ipairs(outfits) do
      p:addOutfit( v )
   end


   --[[
   --    Medium and low slots
   --]]
   outfits  = { }
   -- Medium slots
   i        = 0
   while i < use_medium do
      outfits[ #outfits+1 ] = medium[ rnd.rnd(1,#medium) ]
      i = i + 1
   end
   -- Low slots
   i        = 0
   while i < use_low do
      outfits[ #outfits+1 ] = low[ rnd.rnd(1,#low) ]
      i = i + 1
   end
   -- Add slots
   for k,v in ipairs(outfits) do
      p:addOutfit( v )
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
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:shipSlots()

   -- Defaults
   medium      = { "Civilian Jammer" }
   secondary   = { }
   apu         = { }
   use_primary = rnd.rnd(nhigh) -- Use fewer slots
   use_secondary = 0
   use_medium  = 0
   use_low     = 0


   -- Per ship type
   if shipsize == "small" then
      primary  = { "Laser Cannon", "Plasma Blaster" }
      medium   = { "Civilian Jammer" }
      if rnd.rnd() > 0.8 then
         use_medium = 1
      end
   else
      primary  = { "Laser Turret" }
      medium   = { "Civilian Jammer" }
      if rnd.rnd() > 0.5 then
         use_medium = 1
      end
      low      = { "Plasteel Plating" }
      if rnd.rnd() > 0.5 then
         use_low = 1
      end
   end
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low )
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
         low    = { "Solar Panel" }
      elseif class == "Fighter" then
         high   = { "Laser Cannon", "Plasma Blaster", "40mm Autocannon" }
         p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-1 ) -- Adds uniformity
         p:addOutfit( "Seeker Launcher" )
         high   = { }
         medium = { "Reactor Class I", "Generic Afterburner", "Milspec Jammer",
                    "Auxiliary Processing Unit I" }
         low    = { "Shield Capacitor", "Plasteel Plating", "Engine Reroute" }
      elseif class == "Bomber" then
         high   = { "Laser Cannon", "Plasma Blaster" }
         p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-2 ) -- Adds uniformity
         p:addOutfit( "Seeker Launcher", 2 )
         high   = { }
         medium = { "Reactor Class I", "Generic Afterburner", "Milspec Jammer",
                    "Auxiliary Processing Unit I" }
         low    = { "Shield Capacitor", "Plasteel Plating", "Engine Reroute", "Battery"  }
      end
   elseif shipsize == "medium" then
      high   = { "Laser Turret" }
      p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-2 ) -- Adds uniformity
      p:addOutfit( "Seeker Launcher", 2 )
      medium = { "Reactor Class II", "Generic Afterburner", "Milspec Jammer",
                 "Auxiliary Processing Unit II" }
      low    = { "Shield Capacitor II", "Shield Capacitor III", "Plasteel Plating",
                 "Engine Reroute", "Battery II" }
   else
      high   = { "Laser Turret", "Heavy Ion Turret" }
      p:addOutfit( high[ rnd.rnd(1,#high) ], nhigh-2 ) -- Adds uniformity
      p:addOutfit( "Seeker Launcher", 2 )
      medium = { "Reactor Class III", "Milspec Jammer",
                 "Auxiliary Processing Unit III" }
      low    = { "Shield Capacitor III", "Shield Capacitor IV", "Battery III" }
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



