include("scripts/pilot/generic.lua")


--[[
-- Creates a might pirate to be killed
--]]
function pirate_create( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Choose pirate type
   r = rnd.rnd()
   if r < 0.25 then
      p,o = pirate_createKestrel( pirate_create )
   elseif r < 0.5 then
      p,o = pirate_createAdmonisher( pirate_create )
   elseif r < 0.75 then
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
   pilots   = pilot.add( ship )
   p        = pilots[1]

   -- Remove outfits
   p:rmOutfit( "all" )

   return p
end


-- Gets a generic pirate outfit
function pirate_outfitGeneric( p, o )
   r = rnd.rnd()
   if r < 0.25 then -- Get cannon
      return pirate_outfitCannon( p, o )
   elseif r < 0.5 then -- Get turret
      return pirate_outfitTurret( p, o )
   elseif r < 0.8 then -- Get special weapon
      return pirate_outfitSecondary( p, o )
   else
      return pirate_outfitModification( p, o )
   end
end


-- Tries to add a turret
function pirate_outfitTurret( p, o )
   turrets = {
      "Heavy Ion Turret",
      "Laser Turret"
   }
   return pilot_outfitAdd( p, o, turrets )
end


-- Tries to add a cannon
function pirate_outfitCannon( p, o )
   cannons = {
      "Ripper MK2",
      "Laser Cannon",
      "Ion Cannon",
      "Plasma Blaster", -- Biased towards plasma
      "Plasma Blaster"
   }
   return pilot_outfitAdd( p, o, cannons )
end


-- Tries to add a secondary weapon
function pirate_outfitSecondary( p, o )
   sec = {
      "150mm Railgun",
      { "Seeker Launcher", { "Seeker Missile", 15 } },
      { "Headhunter Launcher", { "Headhunter Missile", 10 } },
      { "Banshee Launcher", { "Banshee Rocket", 30 } },
      { "Mace Launcher", { "Mace Rocket", 7 } },
      { "EMP Grenade Launcher", { "EMP Grenade", 6 } }
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a ranged weapon
function pirate_outfitRanged( p, o )
   sec = {
      { "Seeker Launcher", { "Seeker Missile", 15 } },
      { "Headhunter Launcher", { "Headhunter Missile", 10 } }
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a modification
function pirate_outfitModification( p, o )
   mods = {
      -- Shield
      "Shield Capacitor",
      "Shield Booster",
      -- Armour
      "Plasteel Plating",
      "Plasteel Plating",
      "Nanobond Plating",
      "Droid Repair Crew",
      "Droid Repair Crew",
      -- Energy
      "Solar Panel",
      "Battery",
      "Reactor Class I",
      "Reactor Class I",
      "Reactor Class II",
      -- Movement stuff
      "Engine Reroute",
      "Improved Stabilizer",
      "Steering Thrusters",
      -- Make jammers more likely.
      "Civilian Jammer",
      "Civilian Jammer",
      "Civilian Jammer",
      "Civilian Jammer",
      "Milspec Jammer",
      "Milspec Jammer"
   }
   return pilot_outfitAdd( p, o, mods )
end


-- Creates a pirate flying a "Pirate Kestrel"
function pirate_createKestrel( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   if pirate_create then
      p = pirate_createEmpty( "Pirate Kestrel" )
   else
      p = "Pirate Kestrel"
   end

   -- Kestrel gets some good stuff
   o = {}
   pirate_outfitTurret(p,o)
   pirate_outfitRanged(p,o)
   pirate_outfitModification(p,o)
   -- Might not have enough room for these last three
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)

   return p,o
end


-- Creates a pirate flying a "Pirate Admonisher"
function pirate_createAdmonisher( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   if pirate_create then
      p = pirate_createEmpty( "Pirate Admonisher" )
   else
      p = "Pirate Admonisher"
   end

   -- Make sure Admonisher has at least one cannon
   o = {}
   pirate_outfitCannon(p,o)
   pirate_outfitSecondary(p,o)
   pirate_outfitModification(p,o)
   -- Probably won't have much room left
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)

   return p,o
end


-- Creates a pirate flying a "Pirate Ancestor"
function pirate_createAncestor( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   if pirate_create then
      p = pirate_createEmpty( "Pirate Ancestor" )
   else
      p = "Pirate Ancestor"
   end

   -- Should have at least one cannon and ranged
   o = {}
   pirate_outfitCannon(p,o)
   pirate_outfitRanged(p,o)
   -- Probably won't have much room left
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)

   return p,o
end


-- Ceates a pirate flying a "Pirate Vendetta"
function pirate_createVendetta( pirate_create )
   -- Create by default
   if pirate_create == nil then
      pirate_create = true
   end

   -- Create the pirate ship
   if pirate_create then
      p = pirate_createEmpty( "Pirate Vendetta" )
   else
      p = "Pirate Vendetta"
   end

   -- Biased towards cannons
   o = {}
   pirate_outfitCannon(p,o)
   pirate_outfitCannon(p,o)
   pirate_outfitCannon(p,o)
   -- Probably won't have much room left
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)
   pirate_outfitGeneric(p,o)

   return p,o
end


--[[
-- @brief Generates pilot names
--]]
function pirate_name ()
   descriptors = {
      "The Lustful",
      "The Black",
      "The Red",
      "The Green",
      "The Blue",
      "The Bloody",
      "The Morbid",
      "The Horrible",
      "The Dark",
      "The Evil",
      "Pirate's",
      "Night's",
      "Space's",
      "Astro",
      "Destroyer"
   }
   actors = {
      "Green Beard",
      "Black Beard",
      "Brown Beard",
      "Red Beard",
      "Blue Beard",
      "Corpsebride",
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
      "Executioner",
      "Killer",
      "Thunder",
      "Lance"
   }
   descriptor = descriptors[ rnd.rnd(1,#descriptors) ]
   actor = actors[ rnd.rnd(1,#actors) ]
   return descriptor .. " " .. actor
end
