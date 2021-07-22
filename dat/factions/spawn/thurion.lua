local scom = require "factions.spawn.lib.common"


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
   elseif r < 0.6 then
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Thurion Virtuosity", 45 )
   else
      scom.addPilot( pilots, "Thurion Apprehension", 75 )
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.4 then
      scom.addPilot( pilots, "Thurion Virtuosity", 45 )
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
   elseif r < 0.6 then
      scom.addPilot( pilots, "Thurion Virtuosity", 45 )
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Thurion Taciturnity", 40 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
   else
      scom.addPilot( pilots, "Thurion Apprehension", 75 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
   end

   return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
   local pilots = {}

   -- Generate the capship
   scom.addPilot( pilots, "Thurion Certitude", 140 )

   -- Generate the escorts
   local r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
      scom.addPilot( pilots, "Thurion Perspicacity", 20 )
   elseif r < 0.8 then
      scom.addPilot( pilots, "Thurion Virtuosity", 45 )
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
   else
      scom.addPilot( pilots, "Thurion Apprehension", 75 )
      scom.addPilot( pilots, "Thurion Ingenuity", 25 )
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
   local pilots = scom.spawn( spawn_data, "Thurion" )

   -- Make sure they don't die because of nebula
   local nebu_dens, nebu_vol = system.cur():nebula()
   if nebu_vol > 0 then
      local new_pilots = {}
      for i, s in ipairs(pilots) do
         local dmg = nebu_vol * (1-s.pilot:shipstat("nebu_absorb",true))
         if s.pilot:stats().shield_regen > dmg then
            table.insert( new_pilots, s )
         else
            s.pilot:rm()
         end
      end
      pilots = new_pilots
   end

   -- Unknown until known
   if not faction.get("Thurion"):known() then
      for i, s in ipairs( pilots ) do
         s.pilot:rename(_("Unknown"))
      end
   end

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   -- Case no ship was actually spawned, just create an arbitrary delay
   if #pilots == 0 then
      return 10
   end

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end

