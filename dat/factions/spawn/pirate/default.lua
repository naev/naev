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

local prefer_fleets, hostile_system

local table_patrol = {
   { w=0.3, shyena },
   { w=0.5, sshark },
   { w=0.8, sshark, shyena },
   { svendetta, sshark, shyena },
}
local table_loner_weak = {
   { w=0.3, shyena },
   { w=0.5, sshark },
   { w=0.65, svendetta },
   { w=0.85, sancestor },
   { sphalanx },
}
local table_loner_strong = {
   { w=0.4, srhino },
   { w=0.7, sadmonisher },
   { sstarbridge },
}
local table_squad = {
   { w=0.3, svendetta, sancestor, sancestor, shyena },
   { w=0.5, svendetta, sancestor, sshark, shyena },
   { w=0.7, srhino, sphalanx, sshark },
   { w=0.85, sadmonisher, svendetta, sshark, shyena },
   { sstarbridge, sshark, sshark },
}
local table_capship = {
   { w=0.3, skestrel },
   { w=0.6, skestrel, sadmonisher, svendetta, svendetta },
   { w=0.9, skestrel, sphalanx, svendetta, sancestor, sshark },
   { skestrel, srhino, sadmonisher, svendetta, sancestor, sshark },
}

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( {
      __nofleet = not prefer_fleets and (rnd.rnd() < 0.7),
      __stealth = (hostile_system or (rnd.rnd() < 0.9)),
   }, table_patrol )
end

local function spawn_loner_weak ()
   return scom.doTable( {
      __nofleet = true,
      __stealth = (hostile_system or (rnd.rnd() < 0.7)),
   }, table_loner_weak )
end

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

-- @brief Creation hook.
return function ( t, max, params )
   params = params or {}
   prefer_fleets = params.prefer_fleets

   -- Check to see if it's a hostile system
   hostile_system = false
   for k,v in ipairs(system.cur():spobs()) do
      local f = v:faction()
      if f and f:areEnemies( params.faction ) then
         params.hostile_system = true
         break
      end
   end
   params.hostile_system = hostile_system

   -- Make it harder for large ships to spawn in hostile territory
   local capship_base = -500
   if hostile_system then
      capship_base = -800
   end

   -- Create weights for spawn table
   t.patrol       = { f=spawn_patrol, w=max }
   t.loner_weak   = { f=spawn_loner_weak, w=max }
   t.loner_strong = { f=spawn_loner_strong, w=math.max(0, -80 + 1.0 * max ) }
   t.squad        = { f=spawn_squad, w=math.max(0, -120 + 0.80 * max) }
   t.capship      = { f=spawn_capship, w=math.max(0, capship_base + 1.70 * max) }
end
