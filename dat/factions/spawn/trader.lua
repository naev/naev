local scom = require "factions.spawn.lib.common"


local function add_llama( pilots )
   scom.addPilot( pilots, "Llama", 20, {name=_("Trader Llama")})
end
local function add_koala( pilots )
   scom.addPilot( pilots, "Koala", 30, {name=_("Trader Koala")})
end
local function add_quicksilver( pilots )
   scom.addPilot( pilots, "Quicksilver", 40, {name=_("Trader Quicksilver")})
end
local function add_mule( pilots )
   scom.addPilot( pilots, "Mule", 60, {name=_("Trader Mule")})
end
local function add_rhino( pilots )
   scom.addPilot( pilots, "Rhino", 70, {name=_("Trader Rhino")})
end
local function add_shark( pilots )
   scom.addPilot( pilots, "Shark", 15, {name=_("Trader Shark"), ai="mercenary"})
end


-- @brief Spawns a small trade fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.3 then
      add_llama( pilots )
   elseif r < 0.5 then
      add_koala( pilots )
   elseif r < 0.7 then
      add_llama( pilots )
      add_llama( pilots )
   elseif r < 0.8 then
      add_koala( pilots )
      add_llama( pilots )
      add_llama( pilots )
   elseif r < 0.9 then
      add_quicksilver( pilots)
   elseif r < 0.97 then
      add_mule( pilots)
   else
      add_rhino( pilots)
   end

   return pilots
end


-- @brief Spawns a larger trade fleet
function spawn_squad ()
   local pilots = {}
   if rnd.rnd() < 0.5 then
      pilot.__nofleet = true
   end
   local r = rnd.rnd()

   if r < 0.5 then
      for i=1,rnd.rnd(5,8) do
         if rnd.rnd() < 0.6 then
            add_llama( pilots )
         else
            add_koala( pilots )
         end
      end
      for i=1,rnd.rnd(1,3) do
         add_shark( pilots )
      end
   elseif r < 0.8 then
      add_mule( pilots )
      for i=1,rnd.rnd(3,6) do
         local lr = rnd.rnd()
         if lr < 0.3 then
            add_llama( pilots )
         elseif lr < 0.8 then
            add_koala( pilots )
         else
            add_quicksilver( pilots )
         end
      end
      for i=1,rnd.rnd(1,3) do
         add_shark( pilots )
      end
   else
      for i=1,rnd.rnd(3,5) do
         if rnd.rnd() < 0.65 then
            add_mule( pilots )
         else
            add_rhino( pilots )
         end
      end
   end

   return pilots
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 400
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)

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
   local pilots = scom.spawn( spawn_data, "Trader" )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
