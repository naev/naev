include("scripts/pilot/generic.lua")
include("ai/equip/helper.lua")
include("ai/equip/outfits.lua")


--[[
-- @brief Creates a mighty Empire warship.
--
--    @param empire_create If nil or true actually creates a Empire warship
--           with a random name and all, otherwise it'll give ship type and
--           outfits.
--    @return If empire_create is false it'll return a string containing the
--           name of the ship of the Empire warship and a table containing the
--           outfits, otherwise it'll return the Empire warship itself and the
--           outfit table.
--]]
function empire_create( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Choose warship type
   local z = rnd.rnd()
   local p, o
   local ship_name
   if z < 0.4 then
      p,o = empire_createPeacemaker( empire_create )
      if rnd.rnd() < 0.33 then
         ship_name = empire_namePlanet()
      end
   elseif z < 0.7 then
      p,o = empire_createHawking( empire_create )
      if rnd.rnd() < 0.33 then
         ship_name = empire_namePlanet()
      end
   else
      p,o = empire_createPacifier( empire_create )
   end

   -- Set name if needed
   if ship_name == nil then
      ship_name = empire_name()
   end

   -- Set name
   if empire_create then
      p:rename( ship_name )
   end
   return p,o
end


-- Creates an empty ship for the Empire warship
function empire_createEmpty( ship )
   -- Create the pilot
   local pilots   = pilot.add( ship )
   local p        = pilots[1]

   -- Remove outfits
   p:rmOutfit( "all" )

   return p
end


-- Creates an Empire Peacemaker warship
function empire_createPeacemaker( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Peacemaker" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Peacemaker"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Peacemaker gets some good stuff
   primary        = { "Turbolaser", "Railgun Turret" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   medium         = equip_mediumHig()
   low            = equip_lowHig()
   apu            = equip_apuHig()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end


-- Creates an Empire Hawking warship
function empire_createHawking( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Hawking" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Hawking"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Hawking gets some good stuff
   primary        = { "Turbolaser", "Heavy Laser" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   medium         = equip_mediumHig()
   low            = equip_lowHig()
   apu            = equip_apuHig()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end


-- Creates an Empire Pacifier warship
function empire_createPacifier( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Pacifier" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Pacifier"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Pacifier isn't bad either
   primary        = { "Heavy Ripper Cannon", "Laser Turret MK3" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   medium         = equip_mediumMed()
   low            = equip_lowMed()
   apu            = equip_apuHig()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end

-- Creates an Empire Shark warship
function empire_createShark( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Shark" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Shark"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Shark gets some good stuff
   primary        = { "Laser Cannon MK3", "Ripper Cannon" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh -1
   use_secondary  = 1
   medium         = equip_mediumLow()
   low            = equip_lowLow()
   apu            = equip_apuLow()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end

-- Creates an Empire Lancelot warship
function empire_createLancelot( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Lancelot" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Lancelot"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Lancelot gets some good stuff
   primary        = { "Laser Cannon MK3", "Ripper Cannon" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh-1
   use_secondary  = 1
   medium         = equip_mediumLow()
   low            = equip_lowLow()
   apu            = equip_apuLow()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end

-- Creates an Empire Admonisher warship
function empire_createAdmonisher( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p, s, olist
   if empire_create then
      p     = empire_createEmpty( "Empire Admonisher" )
      s     = p:ship()
      olist = nil
   else
      p     = "Empire Admonisher"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low, apu
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Admonisher gets some good stuff
   primary        = { "Heavy Ripper Cannon", "Ripper Cannon" }
   secondary      = { "Headhunter Launcher" }
   use_primary    = nhigh-1
   use_secondary  = 1
   medium         = equip_mediumMed()
   low            = equip_lowMed()
   apu            = equip_apuMed()

   -- Finally add outfits
   equip_ship( p, true, primary, secondary, medium, low, apu,
               use_primary, use_secondary, use_medium, use_low, olist )

   return p,olist
end

--[[
-- @brief Tries to get a name from an Empire planet.
--]]
function empire_namePlanet ()
   local plt = planet.get( faction.get( "Empire" ) )
   local k   = plt:class()
   if k == "M" or k == "P" or k == "H" or k == "D" then
      return "ESS " .. plt:name()
   end
   return nil
end


--[[
-- @brief Generates pilot names
--]]
function empire_name ()
   local actors = {
      "Warrior",
      "Invincible",
      "Enterprise",
      "Rising Star",
      "Vanguard",
      "Defender",
      "Pride",
      "Glory",
      "Ardent", -- Thanks, British Navy.
      "Argonaut",
      "Ardent",
      "Atlas",
      "Bellerophon",
      "Centurion",
      "Centaur",
      "Colossus",
      "Conqueror",
      "Conquest",
      "Defiance",
      "Defiant",
      "Diligence",
      "Dragon",
      "Dreadnought",
      "Expedition",
      "Foresight",
      "Gladiator",
      "Goliath",
      "Hecate",
      "Hope",
      "Impregnable",
      "Intrepid",
      "Leviathan",
      "Loyalty",
      "Magnificence",
      "Majestic",
      "Minotaur",
      "Orion",
      "Panther",
      "Pegasus",
      "Resolution",
      "Revenge",
      "Robust",
      "Sceptre",
      "Sovereign",
      "Sterling",
      "Tempest",
      "Theseus",
      "Thunderer",
      "Tremendous",
      "Trident",
      "Triumph",
      "Union",
      "Unity",
      "Valiant",
      "Vengeance",
      "Victorious",
      "Victory",
      "Vigilant",
      "Zealous",
      "Cougar", -- Animals
      "Ocelot",
      "Lynx",
      "Bobcat",
      "Serval",
      "Leopard",
      "Lion",
      "Jaguar",
      "Leopard",
      "Tiger",
      "Cheetah",
      "Jackal",
      "Wolf",
      "Coyote",
      "Kodiak",
      "Osprey",
      "Harrier",
      "Falcon",
      "Owl",
      "Hawk",
      "Eagle",
      "Heron",
      "Ham Sandwich", -- Silliness
      "Platypus",
      "Dodo",
      "Turducken",
      "Ridiculous",
      "Rustbucket",
      "Mediocrity",
      "Segfault",
      "Juice Box"
   }
   local actor = actors[ rnd.rnd(1,#actors) ]

   return "ESS " .. actor
end
