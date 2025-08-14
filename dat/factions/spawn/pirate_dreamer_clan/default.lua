local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local shyena      = ship.get("Pirate Hyena")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonisher = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local sbedivere   = ship.get("Pirate Bedivere")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

local prefer_fleets, hostile_system

local table_loner_strong = {
   { w=0.1, sbedivere },
   { w=0.3, sadmonisher },
   { w=0.5, sphalanx },
   { w=0.7, srhino },
   { sstarbridge },
}
local table_squad = {
   { w=0.3, svendetta, sancestor, sancestor, shyena },
   { w=0.5, svendetta, sancestor, var.pirate_shark, shyena },
   { w=0.7, srhino, sbedivere, var.pirate_shark },
   { w=0.85, sadmonisher, svendetta, var.pirate_shark, shyena },
   { sstarbridge, var.pirate_shark, var.pirate_shark },
}

local function spawn_loner_strong ()
   return scom.doTable( {
      __nofleet = true,
      __stealth = (hostile_system or (rnd.rnd() < 0.6)),
   }, table_loner_strong )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.6),
      __stealth = (hostile_system or (rnd.rnd() < 0.6)),
   }, table_squad )
end

return function( t, _max, params )
   prefer_fleets = params.prefer_fleets
   hostile_system = params.hostile_system
   
   -- Overwrite stuff
   t.loner_strong.f = spawn_loner_strong
   t.squad.f = spawn_squad
end, 10
