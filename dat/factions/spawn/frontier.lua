local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Hyena")
local sancestor   = ship.get("Ancestor")
local slancelot   = ship.get("Lancelot")
--local svendetta = ship.get("Vendetta")
local sphalanx    = ship.get("Phalanx")

-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = { __doscans = true }
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
   elseif r < 0.8 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, shyena )
   else
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, shyena )
   end

   return pilots
end

-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   if rnd.rnd() < 0.5 then
      pilots.__doscans = true
   end

   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sphalanx )
      scom.addPilot( pilots, slancelot )
   else
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, sancestor )
   end

   return pilots
end

local ffrontier = faction.get("Frontier")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = 0.33*max

   return scom.init( ffrontier, weights, max, {patrol=true} )
end
