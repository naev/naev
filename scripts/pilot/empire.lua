include("scripts/pilot/generic.lua")


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
   local p
   local o
   local ship_name
   if z < 0.5 then
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


-- Gets a generic empire outfit
function empire_outfitGeneric( p, o )
   local r = rnd.rnd()
   if r < 0.25 then -- Get cannon
      return empire_outfitCannon( p, o )
   elseif r < 0.5 then -- Get turret
      return empire_outfitTurret( p, o )
   elseif r < 0.8 then -- Get special weapon
      return empire_outfitSecondary( p, o )
   else
      return empire_outfitModification( p, o )
   end
end


-- Gets a Warship empire outfit
function empire_outfitWarship( p, o )
   local r = rnd.rnd()
   if r < 0.25 then -- Get turret
      return empire_outfitTurret( p, o )
   elseif r < 0.4 then -- Get special weapon
      return empire_outfitSecondary( p, o )
   else
      return empire_outfitModification( p, o )
   end
end


-- Tries to add a turret
function empire_outfitTurret( p, o )
   local turrets = {
      "Heavy Ion Turret",
      "Laser Turret MK2"
   }
   return pilot_outfitAdd( p, o, turrets )
end


-- Tries to add a cannon
function empire_outfitCannon( p, o )
   local cannons = {
      "Ripper MK2",
      "Laser Cannon MK2",
      "Ion Cannon",
   }
   return pilot_outfitAdd( p, o, cannons )
end


-- Tries to add a secondary weapon
function empire_outfitSecondary( p, o )
   local sec = {
      "150mm Railgun",
      "Headhunter Launcher",
      "EMP Grenade Launcher",
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a ranged weapon
function empire_outfitRanged( p, o )
   local sec = {
      { "Headhunter Launcher", { "Headhunter Missile", 10 } }
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a modification
function empire_outfitModification( p, o )
   local mods = {
      -- Shield
      "Shield Capacitor",
      "Shield Booster",
      -- Armour
      "Plasteel Plating",
      "Nanobond Plating",
      "Droid Repair Crew",
      -- Energy
      "Solar Panel",
      "Battery",
      -- Movement stuff
      "Engine Reroute",
      "Improved Stabilizer",
      "Steering Thrusters",
   }
   return pilot_outfitAdd( p, o, mods )
end

function empire_outfitWarshipBase( p, o )
   local armour = {
      "Plasteel Plating",
      "Nanobond Plating"
   }
   local reactor = {
      "Reactor Class II",
      "Reactor Class III",
   }
   pilot_outfitAdd( p, o, { "Milspec Jammer" } )
   pilot_outfitAdd( p, o, armour )
   pilot_outfitAdd( p, o, reactor )
end


-- Creates an Empire Hawking warship
function empire_createHawking( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p
   if empire_create then
      p = empire_createEmpty( "Empire Hawking" )
   else
      p = "Empire Hawking"
   end

   -- Kestrel gets some good stuff
   local o = {}
   empire_outfitWarshipBase(p,o)
   empire_outfitTurret(p,o)
   empire_outfitTurret(p,o)
   empire_outfitRanged(p,o)
   empire_outfitModification(p,o)
   -- Might not have enough room for these last three
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)
   empire_outfitWarship(p,o)

   return p,o
end


-- Creates an Empire Pacifier warship
function empire_createPacifier( empire_create )
   -- Create by default
   if empire_create == nil then
      empire_create = true
   end

   -- Create the empire ship
   local p
   if empire_create then
      p = empire_createEmpty( "Empire Pacifier" )
   else
      p = "Empire Pacifier"
   end

   -- Make sure Admonisher has at least one cannon
   local o = {}
   empire_outfitCannon(p,o)
   empire_outfitSecondary(p,o)
   empire_outfitModification(p,o)
   -- Probably won't have much room left
   empire_outfitGeneric(p,o)
   empire_outfitGeneric(p,o)
   empire_outfitGeneric(p,o)
   empire_outfitGeneric(p,o)
   empire_outfitGeneric(p,o)
   empire_outfitGeneric(p,o)

   return p,o
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
