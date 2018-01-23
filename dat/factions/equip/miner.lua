-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does miner pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Start with an empty ship
   p:rmOutfit("all")
   p:rmOutfit("cores")

   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )
   local class = p:ship():class()

   -- Cores
   equip_cores(p, equip_getCores(p, shipsize, {
            {"Melendez Ox Engine", "Unicorp PT-100 Core System", "S&K Small Cargo Hull"},
            {"Melendez Ox XL Engine", "Unicorp PT-200 Core System", "S&K Small Cargo Hull"}
         }))

   -- Outfits
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   medium      = { "Asteroid Scanner" }
   use_medium  = 1

   weapons = {}
   use_primary = 1   -- Only 1 asteroid weapon
   use_secondary = 0
   use_medium  = 0
   use_low     = 0

   addWeapons( {"Asterokiller Gauss Gun"}, use_primary )

   equip_ship( p, true, weapons, medium, low,
               use_medium, use_low )

end
