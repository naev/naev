local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Pirate Hyena")
local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonsher  = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

local spir = {}
local hostile_system = false

-- @brief Spawns a small patrol fleet.
function spir.spawn_patrol ()
   local pilots = {}
   pilots.__nofleet = (rnd.rnd() < 0.7)
   pilots.__stealth = hostile_system or (rnd.rnd() < 0.9)
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, shyena )
   elseif r < 0.5 then
      scom.addPilot( pilots, sshark )
   elseif r < 0.8 then
      scom.addPilot( pilots, sshark )
      scom.addPilot( pilots, shyena )
   else
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sshark )
      scom.addPilot( pilots, shyena )
   end

   return pilots
end

function spir.spawn_loner_weak ()
   local pilots = {}
   pilots.__nofleet = true
   pilots.__stealth = hostile_system or (rnd.rnd() < 0.7)

   local r = rnd.rnd()
   if r < 0.3 then
      scom.addPilot( pilots, shyena )
   elseif r < 0.5 then
      scom.addPilot( pilots, sshark )
   elseif r < 0.65 then
      scom.addPilot( pilots, svendetta )
   elseif r < 0.85 then
      scom.addPilot( pilots, sancestor )
   else
      scom.addPilot( pilots, sphalanx )
   end

   return pilots
end

function spir.spawn_loner_strong ()
   local pilots = {}
   pilots.__nofleet = true
   pilots.__stealth = hostile_system or (rnd.rnd() < 0.7)

   local r = rnd.rnd()
   if r < 0.4 then
      scom.addPilot( pilots, srhino )
   elseif r < 0.7 then
      scom.addPilot( pilots, sadmonsher )
   else
      scom.addPilot( pilots, sstarbridge )
   end

   return pilots
end

-- @brief Spawns a medium sized squadron.
function spir.spawn_squad ()
   local pilots = {}
   pilots.__nofleet = (rnd.rnd() < 0.6)
   pilots.__stealth = hostile_system or (rnd.rnd() < 0.7)
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, shyena )
   elseif r < 0.5 then
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sshark )
      scom.addPilot( pilots, shyena )
   elseif r < 0.7 then
      scom.addPilot( pilots, srhino )
      scom.addPilot( pilots, sphalanx )
      scom.addPilot( pilots, sshark )
   elseif r < 0.85 then
      scom.addPilot( pilots, sadmonsher )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sshark )
      scom.addPilot( pilots, shyena )
   else
      scom.addPilot( pilots, sstarbridge )
      scom.addPilot( pilots, sshark )
      scom.addPilot( pilots, sshark )
   end

   return pilots
end

-- @brief Spawns a capship with escorts.
function spir.spawn_capship ()
   local pilots = {}
   pilots.__nofleet = (rnd.rnd() < 0.5)
   pilots.__stealth = hostile_system or (rnd.rnd() < 0.5)
   local r = rnd.rnd()

   -- Generate the capship
   scom.addPilot( pilots, skestrel )

   -- Generate the escorts
   if r < 0.5 then
      scom.addPilot( pilots, sadmonsher )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, svendetta )
   elseif r < 0.8 then
      scom.addPilot( pilots, sphalanx )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sshark )
   else
      scom.addPilot( pilots, srhino )
      scom.addPilot( pilots, sadmonsher )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sshark )
   end

   return pilots
end

-- @brief Creation hook.
function spir.create ( fpirate, max, params )
   params = params or {}
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
