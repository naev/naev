include("scripts/pilot/generic.lua")


--[[
-- @brief Creates a might Empire warship.
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
   z = rnd.rnd()
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
   pilots   = pilot.add( ship )
   p        = pilots[1]

   -- Remove outfits
   p:rmOutfit( "all" )

   return p
end


-- Gets a generic empire outfit
function empire_outfitGeneric( p, o )
   r = rnd.rnd()
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
   r = rnd.rnd()
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
   turrets = {
      "Heavy Ion Turret",
      "Laser Turret MK2"
   }
   return pilot_outfitAdd( p, o, turrets )
end


-- Tries to add a cannon
function empire_outfitCannon( p, o )
   cannons = {
      "Ripper MK2",
      "Laser Cannon MK2",
      "Ion Cannon",
   }
   return pilot_outfitAdd( p, o, cannons )
end


-- Tries to add a secondary weapon
function empire_outfitSecondary( p, o )
   sec = {
      "150mm Railgun",
      { "Headhunter Launcher", { "Headhunter Missile", 10 } },
      { "EMP Grenade Launcher", { "EMP Grenade", 20 } }
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a ranged weapon
function empire_outfitRanged( p, o )
   sec = {
      { "Headhunter Launcher", { "Headhunter Missile", 10 } }
   }
   return pilot_outfitAdd( p, o, sec )
end


-- Tries to add a modification
function empire_outfitModification( p, o )
   mods = {
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
   armour = {
      "Plasteel Plating",
      "Nanobond Plating"
   }
   reactor = {
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
   if empire_create then
      p = empire_createEmpty( "Empire Hawking" )
   else
      p = "Empire Hawking"
   end

   -- Kestrel gets some good stuff
   o = {}
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
   if empire_create then
      p = empire_createEmpty( "Empire Pacifier" )
   else
      p = "Empire Pacifier"
   end

   -- Make sure Admonisher has at least one cannon
   o = {}
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
   plt = planet.get( faction.get( "Empire" ) )
   k   = plt:class()
   if k == "M" or k == "P" or k == "H" or k == "D" then
      return "ESS " .. plt:name()
   end
   return nil
end


--[[
-- @brief Generates pilot names
--]]
function empire_name ()
   actors = {
      "Warrior",
      "Ocelot",
      "Invincible",
      "Enterprise",
      "Rising Star",
      "Vanguard",
      "Defender",
      "Pride",
      "Glory",
      "Segfault"
   }
   actor = actors[ rnd.rnd(1,#actors) ]

   return "ESS " .. actor
end
