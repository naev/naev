-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does Collective pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Start with an empty ship
   p:rmOutfit("all")
   p:rmOutfit("cores")

   -- Currently collective are only drones and all the same
   equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 3701 Core System", "Schafer & Kane Light Stealth Plating")
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end

