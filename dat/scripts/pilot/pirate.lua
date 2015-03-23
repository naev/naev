include("pilot/generic.lua")
include("dat/factions/equip/helper.lua")
include("dat/factions/equip/outfits.lua")


--[[
-- @brief Creates a mighty pirate of epic proportions.
--
--    @param pirate_create If nil or true actually creates a pirate with a
--           random name and all, otherwise it'll give ship type and outfits.
--    @return If pirate_create is false it'll return a string containing the
--           name of the ship of the pirate and a table containing the outfits,
--           otherwise it'll return the pirate itself and the outfit table.
--]]
function pirate_create( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Choose pirate type
   local z = rnd.rnd()
   local p, o
   if z < 0.25 then
      p,o = pirate_createKestrel( pirate_create )
   elseif z < 0.5 then
      p,o = pirate_createAdmonisher( pirate_create )
   elseif z < 0.75 then
      p,o = pirate_createAncestor( pirate_create )
   else
      p,o = pirate_createVendetta( pirate_create )
   end

   -- Set name
   if pirate_create then
      p:rename( pirate_name() )
   end
   return p,o
end


-- Creates an empty ship for the pirate
function pirate_createEmpty( ship )
   -- Create the pilot
   local pilots   = pilot.add( ship )
   local p        = pilots[1]

   -- Remove outfits
   p:rmOutfit( "all" )

   return p
end

-- Creates a pirate flying a "Pirate Kestrel"
function pirate_createKestrel( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   local p, s, olist
   if pirate_create then
      p     = pirate_createEmpty( "Pirate Kestrel" )
      s     = p:ship()
      olist = nil
   else
      p     = "Pirate Kestrel"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Kestrel gets some good stuff
   primary        = { "Heavy Ion Turret", "Razor Turret MK2", "Laser Turret MK2", "Turreted Vulcan Gun" }
   secondary      = { "Unicorp Headhunter Launcher" }
   use_primary    = nhigh-2
   use_secondary  = 2
   addWeapons( primary, use_primary )
   addWeapons( secondary, use_secondary )
   medium         = equip_mediumHig()
   low            = equip_lowHig()

   -- FInally add outfits
   equip_ship( p, true, weapons, medium, low,
         use_medium, use_low, olist )

   return p,olist
end


-- Creates a pirate flying a "Pirate Admonisher"
function pirate_createAdmonisher( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   local p, s, olist
   if pirate_create then
      p     = pirate_createEmpty( "Pirate Admonisher" )
      s     = p:ship()
      olist = nil
   else
      p     = "Pirate Admonisher"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Admonisher specializes in forward-firing weapons.
   primary        = { "Mass Driver MK2", "Plasma Blaster MK2", "Vulcan Gun" }
   secondary      = { "Unicorp Headhunter Launcher", "Unicorp Fury Launcher", "Unicorp Banshee Launcher" }
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


-- Creates a pirate flying a "Pirate Ancestor"
function pirate_createAncestor( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   local p, s, olist
   if pirate_create then
      p     = pirate_createEmpty( "Pirate Ancestor" )
      s     = p:ship()
      olist = nil
   else
      p     = "Pirate Ancestor"
      s     = ship.get(p)
      olist = { }
   end

   -- Equipment vars
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Ancestor specializes in ranged combat.
   primary        = { "Laser Cannon MK1", "Laser Cannon MK2", "Plasma Blaster MK1", "Plasma Blaster MK2", "Razor MK1", "Razor MK2" }
   secondary      = { "Unicorp Fury Launcher" }
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


-- Ceates a pirate flying a "Pirate Vendetta"
function pirate_createVendetta( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   local p, s, olist
   if pirate_create then
      p     = pirate_createEmpty( "Pirate Vendetta" )
      s     = p:ship()
      olist = nil
   else
      p     = "Pirate Vendetta"
      s     = ship.get(p)
      olist = { } 
   end

   -- Equipment vars
   local primary, secondary, medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = s:slots()

   -- Vendettas are all about close-range firepower.
   primary        = { "Plasma Blaster MK1", "Plasma Blaster MK2", "Laser Cannon MK1", "Razor MK2" }
   secondary      = { "Unicorp Fury Launcher", "Unicorp Banshee Launcher" }
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


--[[
-- @brief Generates pilot names
--]]
function pirate_name ()
   local articles = {
      "The",
   }
   local descriptors = {
      "Lustful",
      "Bloody",
      "Morbid",
      "Horrible",
      "Terrible",
      "Very Bad",
      "No Good",
      "Dark",
      "Evil",
      "Murderous",
      "Fearsome",
      "Defiant",
      "Unsightly",
      "Pirate's",
      "Night's",
      "Space",
      "Astro",
      "Delicious",
      "Fearless",
      "Eternal",
      "Mighty"
   }
   local colours = {
      "Red",
      "Green",
      "Blue",
      "Cyan",
      "Black",
      "Brown",
      "Mauve",
      "Crimson",
      "Yellow",
      "Purple"
   }
   local actors = {
      "Beard",
      "Moustache",
      "Neckbeard",
      "Demon",
      "Vengeance",
      "Corsair",
      "Pride",
      "Insanity",
      "Peril",
      "Death",
      "Doom",
      "Raider",
      "Devil",
      "Serpent",
      "Bulk",
      "Killer",
      "Thunder",
      "Tyrant",
      "Lance",
      "Destroyer",
      "Horror",
      "Dread",
      "Blargh",
      "Terror"
   }
   local actorspecials = {
      "Angle Grinder",
      "Belt Sander",
      "Chainsaw",
      "Impact Wrench",
      "Band Saw",
      "Table Saw",
      "Drill Press",
      "Jigsaw",
      "Turret Lathe",
      "Claw Hammer",
      "Rubber Mallet",
      "Squeegee",
      "Pipe Wrench",
      "Bolt Cutter",
      "Staple Gun",
      "Crowbar",
      "Pickaxe",
      "Bench Grinder",
      "Scythe"
   }
   local article = articles[ rnd.rnd(1,#articles) ]
   local descriptor = descriptors[ rnd.rnd(1,#descriptors) ]
   local colour = colours[ rnd.rnd(1,#colours) ]
   local actor = actors[ rnd.rnd(1,#actors) ]
   local actorspecial = actorspecials[ rnd.rnd(1,#actorspecials) ]

   if rnd.rnd() < 0.25 then
      return article .. " " .. actorspecial
   else
      local r = rnd.rnd()
      if r < 0.166 then
         return article .. " " .. actor
      elseif r < 0.333 then
         return colour .. " " .. actor
      elseif r < 0.50 then
         return descriptor .. " " .. actor
      elseif r < 0.666 then
         return article .. " " .. descriptor .. " " .. actor
      elseif r < 0.833 then
         return article .. " " .. colour .. " " .. actor
      else
         return article .. " " .. descriptor .. " " .. colour .. " " .. actor
      end
   end
end
