local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Pirate Hyena")
local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonisher = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

local spir = {}
local hostile_system = false
local prefer_fleets = false

spir.table_patrol = {
   { w=0.3, shyena },
   { w=0.5, sshark },
   { w=0.8, sshark, shyena },
   { svendetta, sshark, shyena },
}
spir.table_loner_weak = {
   { w=0.3, shyena },
   { w=0.5, sshark },
   { w=0.65, svendetta },
   { w=0.85, sancestor },
   { sphalanx },
}
spir.table_loner_strong = {
   { w=0.4, srhino },
   { w=0.7, sadmonisher },
   { sstarbridge },
}
spir.table_squad = {
   { w=0.3, svendetta, sancestor, sancestor, shyena },
   { w=0.5, svendetta, sancestor, sshark, shyena },
   { w=0.7, srhino, sphalanx, sshark },
   { w=0.85, sadmonisher, svendetta, sshark, shyena },
   { sstarbridge, sshark, sshark },
}
spir.table_capship = {
   { w=0.3, skestrel },
   { w=0.6, skestrel, sadmonisher, svendetta, svendetta },
   { w=0.9, skestrel, sphalanx, svendetta, sancestor, sshark },
   { skestrel, srhino, sadmonisher, svendetta, sancestor, sshark },
}

function spir.spawn_table( pilots, tbl )
   local r = rnd.rnd()
   for k,t in ipairs(tbl) do
      if not t.w or t.w <= r then
         for i,p in ipairs(t) do
            scom.addPilot( pilots, p )
         end
      end
   end
   return pilots
end

-- @brief Spawns a small patrol fleet.
function spir.spawn_patrol ()
   return spir.spawn_table( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.7),
      __stealth = (hostile_system or (rnd.rnd() < 0.9)),
   }, spir.table_patrol )
end

function spir.spawn_loner_weak ()
   return spir.spawn_table( {
      __nofleet = true,
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, spir.table_loner_weak )
end

function spir.spawn_loner_strong ()
   return spir.spawn_table( {
      __nofleet = true,
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, spir.table_loner_strong )
end

-- @brief Spawns a medium sized squadron.
function spir.spawn_squad ()
   return spir.spawn_table( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.6),
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, spir.table_squad )
end

-- @brief Spawns a capship with escorts.
function spir.spawn_capship ()
   return spir.spawn_table( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.5),
      __stealth = (hostile_system or (rnd.rnd() < 0.5)),
   }, spir.table_capship )
end

-- @brief Creation hook.
function spir.create ( fpirate, max, params )
   params = params or {}
   prefer_fleets = params.prefer_fleets
   local weights = {}

   -- Check to see if it's a hostile system
   hostile_system = false
   for k,v in ipairs(system.cur():spobs()) do
      local f = v:faction()
      if f and f:areEnemies( fpirate ) then
         hostile_system = true
         break
      end
   end

   -- Make it harder for large ships to spawn in hostile territory
   local capship_base = -500
   if hostile_system then
      capship_base = -800
   end

   -- Create weights for spawn table
   weights[ spir.spawn_patrol  ] = max
   weights[ spir.spawn_loner_weak ] = max
   weights[ spir.spawn_loner_strong ] = math.max(0, -80 + 1.0 * max )
   weights[ spir.spawn_squad   ] = math.max(0, -120 + 0.80 * max)
   weights[ spir.spawn_capship ] = math.max(0, capship_base + 1.70 * max)

   -- Allow reweighting
   if params.reweight then
      weights = params.reweight( weights )
   end

   return scom.init( fpirate, weights, max )
end

return spir
