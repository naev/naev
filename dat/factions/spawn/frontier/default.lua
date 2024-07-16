local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Hyena")
local sancestor   = ship.get("Ancestor")
local slancelot   = ship.get("Lancelot")
--local svendetta = ship.get("Vendetta")
local sphalanx    = ship.get("Phalanx")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.5, slancelot },
      { w=0.8, slancelot, shyena },
      { sancestor, shyena },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.5, sphalanx, slancelot },
      { slancelot, slancelot, sancestor },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 100 }
   t.squad   = { f = spawn_squad,   w = 0.33*max }
end, 10
