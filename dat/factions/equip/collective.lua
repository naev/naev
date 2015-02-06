-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- New script to equip the Collective drone swarm. Code probably crap.
--
--    TODO: Give Heavy Drones a proper, unique, and varied weapon loadout. Creativity is weak right now. Bleh.
--		~Areze
--]]
function equip( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "military" then
      equip_collectiveMilitary( p, shipsize )
   else
      equip_generic( p )
   end
end


-- CANNONS
function equip_CollectiveDrone ()
   return { "Neutron Disruptor" }
end
function equip_CollectiveDroneHvy ()
   return { "Heavy Neutron Disruptor" }
end


--[[
-- @Equips a Collective Drone
--]]
function equip_collectiveMilitary( p, shipsize )


   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Drone
      if class == "Drone" then
         equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 3701 Core System", "S&K Light Stealth Plating")
         use_primary    = nhigh
         addWeapons( equip_CollectiveDrone(), use_primary )
      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      
      -- Heavy Drone
      if class == "Heavy Drone" then
         equip_cores(p, "Tricon Cyclone Engine", "Milspec Orion 5501 Core System", "S&K Medium Combat Plating")
         use_primary    = nhigh
         addWeapons( equip_CollectiveDroneHvy(), use_primary )
      end

end
end
