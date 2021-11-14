local scom = require "factions.spawn.lib.common"

local sllama      = ship.get("Llama")
local skoala      = ship.get("Koala")
local squicksilver= ship.get("Quicksilver")
local smule       = ship.get("Mule")
local srhino      = ship.get("Rhino")
local sshark      = ship.get("Shark")

local function add_llama( pilots )
   scom.addPilot( pilots, sllama, {name=_("Trader Llama")})
end
local function add_koala( pilots )
   scom.addPilot( pilots, skoala, {name=_("Trader Koala")})
end
local function add_quicksilver( pilots )
   scom.addPilot( pilots, squicksilver, {name=_("Trader Quicksilver")})
end
local function add_mule( pilots )
   scom.addPilot( pilots, smule, {name=_("Trader Mule")})
end
local function add_rhino( pilots )
   scom.addPilot( pilots, srhino, {name=_("Trader Rhino")})
end
local function add_shark( pilots )
   scom.addPilot( pilots, sshark, {name=_("Trader Shark"), ai="mercenary"})
end

-- Doubles the credits of the pilot
local function double_credits( p )
   p:credits( p:credits() )
end

-- @brief Spawns a small trade fleet.
local function spawn_loner ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.3 then
      add_llama( pilots )
   elseif r < 0.5 then
      add_koala( pilots )
   elseif r < 0.7 then
      add_quicksilver( pilots )
   elseif r < 0.85 then
      add_mule( pilots )
   else
      add_rhino( pilots )
   end

   return pilots
end

local function spawn_fleet_small ()
   local pilots = {}

   for i=1,rnd.rnd(2,5) do
      local r = rnd.rnd()
      if r < 0.5 then
         add_llama( pilots )
      elseif r < 0.8 then
         add_koala( pilots )
      else
         add_quicksilver( pilots )
      end
   end

   return pilots
end

local function spawn_fleet_small_guarded ()
   local pilots = {}

   -- Base Fleet
   for i=1,rnd.rnd(2,4) do
      local r = rnd.rnd()
      if r < 0.5 then
         add_llama( pilots )
      elseif r < 0.8 then
         add_koala( pilots )
      else
         add_quicksilver( pilots )
      end
   end

   -- Give more money
   for k,p in ipairs(pilots) do
      p.postprocess = double_credits
   end

   -- Some Guards
   for i=1,rnd.rnd(1,3) do
      add_shark( pilots )
   end

   return pilots
end


local function spawn_fleet_med ()
   local pilots = {}

   -- Leader
   local mule_leader = false
   if rnd.rnd() < 0.6 then
      add_mule( pilots )
      mule_leader = true
   else
      add_rhino( pilots )
   end

   -- Determine type of fleet (small or large ships)
   if rnd.rnd() < 0.5 then
      for i=2,4 do
         local r = rnd.rnd()
         if r < 0.3 then
            add_llama( pilots )
         elseif r < 0.8 then
            add_koala( pilots )
         else
            add_quicksilver( pilots )
         end
      end
   else
      for i=1,2 do
         if mule_leader or rnd.rnd() < 0.6 then
            add_mule( pilots )
         else
            add_rhino( pilots )
         end
      end
   end

   -- Some Guards
   for i=1,rnd.rnd(3,5) do
      add_shark( pilots )
   end

   return pilots
end

local function spawn_fleet_med_guarded ()
   local pilots = spawn_fleet_med ()

   -- Give more money
   for k,p in ipairs(pilots) do
      p.postprocess = double_credits
   end

   -- Some Guards
   for i=1,rnd.rnd(3,5) do
      add_shark( pilots )
   end

   return pilots
end

local ftrader = faction.get("Trader")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Hostiles (namely pirates atm)
   local host = 0
   local total = 0
   local csys = system.cur()
   for f,v in pairs(csys:presences()) do
      if ftrader:areEnemies(f) then
         host = host + v
      end
      total = total + v
   end
   local hostnorm = host / total

   -- Hermite interpolation
   hostnorm = math.pow(hostnorm,2) * (3-hostnorm)

   -- Create weights for spawn table
   weights[ spawn_loner  ] = 400
   weights[ spawn_fleet_small ] = math.max(1, -150, max ) * (1-hostnorm)
   weights[ spawn_fleet_small_guarded ] = math.max(1, -200, max ) * hostnorm
   weights[ spawn_fleet_med ] = math.max(1, -300 + max) * (1-hostnorm)
   weights[ spawn_fleet_med_guarded ] = math.max(1, -300 + max) * (1-hostnorm)

   return scom.init( ftrader, weights, max )
end
