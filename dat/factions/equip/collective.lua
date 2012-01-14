-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- @brief Does Collective pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Currently collective are only drones and all the same
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end

