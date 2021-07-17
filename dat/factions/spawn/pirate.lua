local scom = require "factions.spawn.lib.common"

local function add_hyena( pilots )
   scom.addPilot( pilots, "Hyena", 15, {name=_("Pirate Hyena")})
end

-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   if rnd.rnd() < 0.5 then
      pilots.__nofleet = true
   end
   local r = rnd.rnd()

   if r < 0.3 then
      add_hyena( pilots )
   elseif r < 0.5 then
      scom.addPilot( pilots, "Pirate Shark", 20 )
   elseif r < 0.8 then
      add_hyena( pilots )
      scom.addPilot( pilots, "Pirate Shark", 20 )
   else
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      add_hyena( pilots )
      scom.addPilot( pilots, "Pirate Shark", 20 )
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   if rnd.rnd() < 0.4 then
      pilots.__nofleet = true
   end
   local r = rnd.rnd()

   if r < 0.4 then
      add_hyena( pilots )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Ancestor", 20 )
      scom.addPilot( pilots, "Pirate Ancestor", 20 )
   elseif r < 0.6 then
      add_hyena( pilots )
      scom.addPilot( pilots, "Pirate Shark", 20 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Ancestor", 20 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Pirate Shark", 20 )
      scom.addPilot( pilots, "Pirate Rhino", 35 )
      scom.addPilot( pilots, "Pirate Phalanx", 45 )
   else
      add_hyena( pilots )
      scom.addPilot( pilots, "Pirate Shark", 20 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Admonisher", 45 )
   end

   return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
   local pilots = {}
   if rnd.rnd() < 0.3 then
      pilots.__nofleet = true
   end
   local r = rnd.rnd()

   -- Generate the capship
   scom.addPilot( pilots, "Pirate Kestrel", 125 )

   -- Generate the escorts
   if r < 0.5 then
      scom.addPilot( pilots, "Pirate Admonisher", 45 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Pirate Phalanx", 45 )
      scom.addPilot( pilots, "Pirate Shark", 20 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Ancestor", 20 )
   else
      scom.addPilot( pilots, "Pirate Rhino", 35 )
      scom.addPilot( pilots, "Pirate Admonisher", 45 )
      scom.addPilot( pilots, "Pirate Shark", 20 )
      scom.addPilot( pilots, "Pirate Vendetta", 25 )
      scom.addPilot( pilots, "Pirate Ancestor", 20 )
   end

   return pilots
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

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
   local pilots = scom.spawn( spawn_data, "Pirate", true )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
