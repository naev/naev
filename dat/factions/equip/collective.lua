-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does Collective pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Currently collective are only drones and all the same
   equip_cores(p, "Tricon Naga Mk9 Engine", "Milspec Orion 5501 Core System", "Schafer & Kane Light Stealth Plating")
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end

