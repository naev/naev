local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local salpaca     = ship.get("Alpaca")
local srhino      = ship.get("Rhino")

local function add_alpaca( pilots )
   scom.addPilot( pilots, salpaca, {name=_("Trader Alpaca")})
end
local function add_llama( pilots )
   scom.addPilot( pilots, var.llama, {name=_("Trader Llama")})
end
local function add_koala( pilots )
   scom.addPilot( pilots, var.koala, {name=_("Trader Koala")})
end
local function add_quicksilver( pilots )
   scom.addPilot( pilots, var.quicksilver, {name=_("Trader Quicksilver")})
end
local function add_mule( pilots )
   scom.addPilot( pilots, var.mule, {name=_("Trader Mule")})
end
local function add_rhino( pilots )
   scom.addPilot( pilots, srhino, {name=_("Trader Rhino")})
end
local function add_shark( pilots )
   scom.addPilot( pilots, var.shark, {name=_("Trader Shark"), ai="mercenary"})
end

-- Doubles the credits of the pilot
local function double_credits( p )
   p:credits( p:credits() )
end

-- @brief Spawns a small trade fleet.
local function spawn_loner ()
   local pilots = {}

   -- Regular loners
   local r = rnd.rnd() -- New random number
   if r < 0.2 then
      add_llama( pilots )
   elseif r < 0.3 then
      add_alpaca( pilots )
   elseif r < 0.5 then
      add_koala( pilots )
   elseif r < 0.8 then
      add_quicksilver( pilots )
   elseif r < 0.9 then
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
      if r < 0.2 then
         add_alpaca( pilots )
      elseif r < 0.5 then
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
      if r < 0.2 then
         add_alpaca( pilots )
      elseif r < 0.5 then
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

local ftrader = faction.get("Free Trader")
return function ( t, max )
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
   t.loner = { f=spawn_loner, w=400 } -- codespell:ignore loner
   t.fleet_small = { f=spawn_fleet_small, w=math.max(1, -150, max ) * (1-hostnorm) }
   t.fleet_small_guarded = { f=spawn_fleet_small_guarded, w=math.max(1, -200, max ) * hostnorm }
end, 10
