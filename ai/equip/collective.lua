--[[
-- @brief Does Collective pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_collective( p )
   -- Currently collective are only drones and all the same
   equip_fillSlots( p, { "Neutron Disruptor" }, { }, { } )
end

