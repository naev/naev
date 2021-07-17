local scom = require "factions.spawn.lib.common"

function spawn_advert ()
   local pilots = {}
   local civships = {
         {"Schroedinger", 8},
         {"Llama", 8},
         {"Gawain", 8},
         {"Hyena", 13}
   }
   local shp = civships[ rnd.rnd(1, #civships) ]
   scom.addPilot( pilots, shp[1], shp[2], {ai="advertiser"} )
   return pilots
end


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, "Schroedinger", 12 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Schroedinger", 12 )
      scom.addPilot( pilots, "Gawain", 7 )
   else
      scom.addPilot( pilots, "Schroedinger", 12 )
      scom.addPilot( pilots, "Schroedinger", 12 )
      scom.addPilot( pilots, "Gawain", 7 )
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, "Schroedinger", 5 )
      scom.addPilot( pilots, "Gawain", 5 )
      scom.addPilot( pilots, "Gawain", 5 )
      scom.addPilot( pilots, "Hyena", 10 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Gawain", 5 )
      scom.addPilot( pilots, "Gawain", 5 )
      scom.addPilot( pilots, "Gawain", 5 )
      scom.addPilot( pilots, "Hyena", 10 )
      scom.addPilot( pilots, "Hyena", 10 )
   else
      scom.addPilot( pilots, "Hyena", 10 )
      scom.addPilot( pilots, "Hyena", 10 )
      scom.addPilot( pilots, "Hyena", 10 )
      scom.addPilot( pilots, "Hyena", 10 )
   end

   return pilots
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -100 + 1.00 * max)

   -- Compute advertisements
   local host = 0
   local csys = system.cur()
   local find = faction.get("Independent")
   for k,fact in pairs(find:enemies()) do
      host = host + csys:presence(fact)
   end
   host = host / csys:presence(find)
   -- The more hostiles, the less advertisers
   -- The modifier should be 0.15 at 10% hostiles, 0.001 at 100% hostiles, and
   -- 1 at 0% hostiles
   weights[ spawn_advert  ] = 30 * math.exp(-host*5)

   -- Create spawn table base on weights
   spawn_table = scom.createSpawnTable( weights )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Spawning hook
function spawn ( presence, max )
   -- Over limit
   if presence > max then
      return 5
   end

   -- Actually spawn the pilots
   local pilots = scom.spawn( spawn_data, "Independent" )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
