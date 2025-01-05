local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonisher = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local skestrel    = ship.get("Pirate Kestrel")
local szebra      = ship.get("Pirate Zebra")

local prefer_fleets, hostile_system

local table_capship = {
   { w=0.1, szebra },
   { w=0.3, skestrel },
   { w=0.5, skestrel, sadmonisher, svendetta, svendetta },
   { w=0.7, szebra, sphalanx, sancestor },
   { w=0.8, skestrel, sphalanx, svendetta, sancestor, var.pirate_shark },
   { w=0.9, skestrel, srhino, sadmonisher, svendetta, sancestor, var.pirate_shark },
   { szebra, sadmonisher, svendetta, sancestor, var.pirate_shark },
}

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   return scom.doTable( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.5),
      __stealth = (hostile_system or (rnd.rnd() < 0.5)),
   }, table_capship )
end

return function( t, _max, params )
   prefer_fleets = params.prefer_fleets
   hostile_system = params.hostile_system

   -- Overwrite stuff
   t.capship.f = spawn_capship
end, 10
