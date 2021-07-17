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
function spawn_solitary_civilians ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, "Llama", 5 )
   elseif r < 0.55 then
      scom.addPilot( pilots, "Hyena", 7 )
   elseif r < 0.75 then
      scom.addPilot( pilots, "Gawain", 7 )
   elseif r < 0.9 then
      scom.addPilot( pilots, "Schroedinger", 12 )
   else
      scom.addPilot( pilots, "Koala", 20 )
   end

   return pilots
end

function spawn_bounty_hunter( shiplist )
   local pilots = {}
   local r = rnd.rnd()
   local params = {name=_("Bounty Hunter"), ai="mercenary"}
   local shp = shiplist[ rnd.rnd(1,#shiplist) ]
   scom.addPilot( pilots, shp[1], shp[2], params )
   return pilots
end


function spawn_bounty_hunter_sml ()
   return spawn_bounty_hunter{
      {"Hyena", 10},
      {"Shark", 20},
      {"Lancelot", 25},
      {"Vendetta", 25},
      {"Ancestor", 20},
   }
end
function spawn_bounty_hunter_med ()
   return spawn_bounty_hunter{
      {"Admonisher", 45},
      {"Phalanx", 45},
      {"Vigilance", 70},
      {"Pacifier", 70},
   }
end
function spawn_bounty_hunter_lrg ()
   return spawn_bounty_hunter{
      {"Kestrel", 90},
      {"Hawking", 105},
      {"Goddard", 120},
   }
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Hostiles (namely pirates atm)
   local host = 0
   local csys = system.cur()
   local find = faction.get("Independent")
   for k,fact in pairs(find:enemies()) do
      host = host + csys:presence(fact)
   end
   local hostnorm = host / csys:presence(find)

   -- Create weights for spawn table
   weights[ spawn_solitary_civilians ] = max
   weights[ spawn_bounty_hunter_sml  ] = math.min( 0.3*max, 50 )
   weights[ spawn_bounty_hunter_med  ] = math.min( 0.2*max, math.max(1, -100 + host ) )
   weights[ spawn_bounty_hunter_lrg  ] = math.min( 0.1*max, math.max(1, -200 + host ) )
   -- The more hostiles, the less advertisers
   -- The modifier should be 0.15 at 10% hostiles, 0.001 at 100% hostiles, and
   -- 1 at 0% hostiles
   weights[ spawn_advert  ] = 30 * math.exp(-hostnorm*5)

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
