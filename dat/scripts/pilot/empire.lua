include("pilot/generic.lua")
include("dat/factions/equip/helper.lua")
include("dat/factions/equip/outfits.lua")


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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Peacemaker gets some good stuff
   primary        = { "Turbolaser", "Railgun Turret" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumHig()
   low            = equip_lowHig()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Hawking gets some good stuff
   primary        = { "Turbolaser", "Heavy Laser" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumHig()
   low            = equip_lowHig()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Pacifier isn't bad either
   primary        = { "Heavy Ripper Cannon", "Laser Turret MK3" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumMed()
   low            = equip_lowMed()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Shark gets some good stuff
   primary        = { "Laser Cannon MK3", "Ripper Cannon" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh -1
   use_secondary  = 1
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumLow()
   low            = equip_lowLow()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Lancelot gets some good stuff
   primary        = { "Laser Cannon MK3", "Ripper Cannon" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-1
   use_secondary  = 1
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumLow()
   low            = equip_lowLow()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

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
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()
   weapons = {}

   -- Admonisher gets some good stuff
   primary        = { "Heavy Ripper Cannon", "Ripper Cannon" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-1
   use_secondary  = 1
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumMed()
   low            = equip_lowMed()

   -- Finally add outfits
   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low, olist )

   return p,olist
end

--[[
-- @brief Tries to get a name from an Empire planet.
--]]
function empire_namePlanet ()
   local plt = planet.get( faction.get( "Empire" ) )
   local k   = plt:class()
   if k == "M" or k == "P" or k == "H" or k == "D" then
      return string.format(_("ESS %s"), _(plt:name()))
   end
   return nil
end


--[[
-- @brief Generates pilot names
--]]
function empire_name ()
   local actors = {
      _("Warrior"),
      _("Invincible"),
      _("Enterprise"),
      _("Rising Star"),
      _("Vanguard"),
      _("Defender"),
      _("Pride"),
      _("Glory"),
      _("Ardent"), -- Thanks, British Navy.
      _("Argonaut"),
      _("Ardent"),
      _("Atlas"),
      _("Bellerophon"),
      _("Centurion"),
      _("Centaur"),
      _("Colossus"),
      _("Conqueror"),
      _("Conquest"),
      _("Defiance"),
      _("Defiant"),
      _("Diligence"),
      _("Dragon"),
      _("Dreadnought"),
      _("Expedition"),
      _("Foresight"),
      _("Gladiator"),
      _("Goliath"),
      _("Hecate"),
      _("Hope"),
      _("Impregnable"),
      _("Intrepid"),
      _("Leviathan"),
      _("Loyalty"),
      _("Magnificence"),
      _("Majestic"),
      _("Minotaur"),
      _("Orion"),
      _("Panther"),
      _("Pegasus"),
      _("Resolution"),
      _("Revenge"),
      _("Robust"),
      _("Sceptre"),
      _("Sovereign"),
      _("Sterling"),
      _("Tempest"),
      _("Theseus"),
      _("Thunderer"),
      _("Tremendous"),
      _("Trident"),
      _("Triumph"),
      _("Union"),
      _("Unity"),
      _("Valiant"),
      _("Vengeance"),
      _("Victorious"),
      _("Victory"),
      _("Vigilant"),
      _("Zealous"),
      _("Cougar"), -- Animals
      _("Ocelot"),
      _("Lynx"),
      _("Bobcat"),
      _("Serval"),
      _("Leopard"),
      _("Lion"),
      _("Jaguar"),
      _("Leopard"),
      _("Tiger"),
      _("Cheetah"),
      _("Jackal"),
      _("Wolf"),
      _("Coyote"),
      _("Kodiak"),
      _("Osprey"),
      _("Harrier"),
      _("Falcon"),
      _("Owl"),
      _("Hawk"),
      _("Eagle"),
      _("Heron"),
      _("Ham Sandwich"), -- Silliness
      _("Platypus"),
      _("Dodo"),
      _("Turducken"),
      _("Ridiculous"),
      _("Rustbucket"),
      _("Mediocrity"),
      _("Segfault"),
      _("Juice Box")
   }
   local actor = actors[ rnd.rnd(1,#actors) ]

   return string.format( _("ESS %s"), actor )
end
