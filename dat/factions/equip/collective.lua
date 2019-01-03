-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")

--[[
-- New script to equip the Collective drone swarm. Code probably crap.
--
--    TODO: Give Heavy Drones a proper, unique, and varied weapon loadout. Creativity is weak right now. Bleh.
--      ~Areze
--]]
function equip( p )
   -- Get ship info
   local shiptype, shipsize = equip_getShipBroad( p:ship():class() )

   -- Split by type
   if shiptype == "robotic" then
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
function equip_CollectiveDroneHvyRanged ()
   return { "Electron Burst Cannon" }
end

--[[
-- @Equips a Collective Drone
--]]
function equip_collectiveMilitary( p, shipsize )
   local medium, low
   local use_primary, use_medium, use_low
   local nhigh, nmedium, nlow = p:ship():slots()

   -- Defaults
   weapons    = {}
   medium     = {}
   low        = {}
   use_medium = 0
   use_low    = 0
   scramble   = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()

      -- Drone
      if class == "Drone" then
         equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Light Stealth Plating")
         use_primary    = nhigh
         addWeapons( equip_CollectiveDrone(), use_primary )

      -- Heavy Drone
      elseif class == "Heavy Drone" then
         equip_cores(p, "Tricon Zephyr II Engine", "Milspec Orion 3701 Core System", "S&K Light Combat Plating")

         -- TODO: Remove assumptions about slot sizes.
         addWeapons( equip_CollectiveDroneHvy(), 2 )
         addWeapons( equip_CollectiveDroneHvyRanged(), 2 )
      end
   else
      warn(string.format("Collective equipper doesn't support '%s' ships",
            shipsize))
      return
   end

   equip_ship( p, scramble, weapons, medium, low, use_medium, use_low )
end
