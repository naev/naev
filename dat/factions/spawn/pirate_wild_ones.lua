local spir = require "factions.spawn.lib.pirate"

local shyena      = ship.get("Pirate Hyena")
local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sadmonisher = ship.get("Pirate Admonisher")
local srevenant   = ship.get("Pirate Revenant")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

-- Overwrite some tables
spir.table_loner_strong = {
   { w=0.4, srevenant },
   { w=0.7, sadmonisher },
   { sstarbridge },
}
spir.table_squad = {
   { w=0.3, svendetta, sancestor, sancestor, shyena },
   { w=0.5, svendetta, sancestor, sshark, shyena },
   { w=0.6, srevenant, sshark, shyena },
   { w=0.75, srevenant, svendetta, sshark },
   { w=0.9, sadmonisher, svendetta, sshark, shyena },
   { sstarbridge, sshark, sshark },
}
spir.table_capship = {
   { w=0.3, skestrel },
   { w=0.6, skestrel, sadmonisher, svendetta, sshark },
   { w=0.9, skestrel, srevenant, svendetta, sancestor, sshark },
   { skestrel, srevenant, sadmonisher, svendetta, sancestor, sshark },
}

local fwildones = faction.get("Wild Ones")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fwildones, max )
end
