local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Hyena")
local sancestor   = ship.get("Ancestor")
local stristan    = ship.get("Tristan")
local sbedivere   = ship.get("Bedivere")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.3, stristan },
      { w=0.5, sancestor },
      { w=0.8, stristan, shyena },
      { stristan, stristan },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.4, sbedivere, stristan },
      { w=0.7, sbedivere, stristan, stristan },
      { stristan, stristan, sancestor },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 100 }
   t.squad   = { f = spawn_squad,   w = 0.33*max }
end, 10
