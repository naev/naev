local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Pirate Hyena")
local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sadmonisher = ship.get("Pirate Admonisher")
local srevenant   = ship.get("Pirate Revenant")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

local prefer_fleets, hostile_system

-- Overwrite some tables
local table_loner_strong = {
   { w=0.4, srevenant },
   { w=0.7, sadmonisher },
   { sstarbridge },
}
local table_squad = {
   { w=0.3, svendetta, sancestor, sancestor, shyena },
   { w=0.5, svendetta, sancestor, sshark, shyena },
   { w=0.6, srevenant, sshark, shyena },
   { w=0.75, srevenant, svendetta, sshark },
   { w=0.9, sadmonisher, svendetta, sshark, shyena },
   { sstarbridge, sshark, sshark },
}
local table_capship = {
   { w=0.3, skestrel },
   { w=0.6, skestrel, sadmonisher, svendetta, sshark },
   { w=0.9, skestrel, srevenant, svendetta, sancestor, sshark },
   { skestrel, srevenant, sadmonisher, svendetta, sancestor, sshark },
}

local function spawn_loner_strong ()
   return scom.doTable( {
      __nofleet = true,
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, table_loner_strong )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.6),
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, table_squad )
end

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
   t.loner_strong.f = spawn_loner_strong
   t.squad.f = spawn_squad
   t.capship.f = spawn_capship
end, 10
