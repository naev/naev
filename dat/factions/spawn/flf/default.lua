local scom = require "factions.spawn.lib.common"

local stristan   = ship.get("Tristan")
local svendetta  = ship.get("Vendetta")
local spacifier  = ship.get("Pacifier") -- codespell:ignore spacifier
local sbedivere  = ship.get("Bedivere")

-- @brief Spawns a small fleet.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { w=0.5, stristan, stristan },
      { w=0.8, stristan, svendetta },
      { stristan, stristan, svendetta },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {}, {
      { w=0.3, spacifier, stristan }, -- codespell:ignore spacifier
      { w=0.5, stristan, stristan, svendetta },
      { w=0.8, stristan, svendetta, svendetta },
      { sbedivere, stristan },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
end, 10
