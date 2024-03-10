local spir = require "factions.spawn.lib.pirate"

local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonisher = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local skestrel    = ship.get("Pirate Kestrel")
local szebra      = ship.get("Pirate Zebra")

spir.table_capship = {
   { w=0.1, szebra },
   { w=0.3, skestrel },
   { w=0.5, skestrel, sadmonisher, svendetta, svendetta },
   { w=0.7, szebra, sphalanx, sancestor },
   { w=0.8, skestrel, sphalanx, svendetta, sancestor, sshark },
   { w=0.9, skestrel, srhino, sadmonisher, svendetta, sancestor, sshark },
   { szebra, sadmonisher, svendetta, sancestor, sshark },
}

local fravenclan = faction.get("Raven Clan")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fravenclan, max )
end
