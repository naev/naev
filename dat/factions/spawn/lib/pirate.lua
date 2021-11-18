local scom = require "factions.spawn.lib.common"

local shyena      = ship.get("Hyena")
local sshark      = ship.get("Pirate Shark")
local svendetta   = ship.get("Pirate Vendetta")
local sancestor   = ship.get("Pirate Ancestor")
local sphalanx    = ship.get("Pirate Phalanx")
local sadmonsher  = ship.get("Pirate Admonisher")
local srhino      = ship.get("Pirate Rhino")
local sstarbridge = ship.get("Pirate Starbridge")
local skestrel    = ship.get("Pirate Kestrel")

local spir = {}
function spir.add_hyena( pilots )
   scom.addPilot( pilots, shyena, {name=_("Pirate Hyena")})
end

-- @brief Spawns a small patrol fleet.
function spir.spawn_patrol ()
   local pilots = {}
   pilots.__nofleet = (rnd.rnd() < 0.7)
   pilots.__stealth = (rnd.rnd() < 0.9)
   local r = rnd.rnd()

   if r < 0.3 then
      spir.add_hyena( pilots )
   elseif r < 0.5 then
      scom.addPilot( pilots, sshark )
   elseif r < 0.8 then
      scom.addPilot( pilots, sshark )
      spir.add_hyena( pilots )
   else
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sshark )
      spir.add_hyena( pilots )
   end

   return pilots
end


function spir.spawn_loner ()
   local pilots = {}
   pilots.__nofleet = true
   pilots.__stealth = (rnd.rnd() < 0.7)

   local r = rnd.rnd()
   if r < 0.2 then
      spir.add_hyena( pilots )
   elseif r < 0.3 then
      scom.addPilot( pilots, sshark )
   elseif r < 0.4 then
      scom.addPilot( pilots, svendetta )
   elseif r < 0.55 then
      scom.addPilot( pilots, sancestor )
   elseif r < 0.7 then
      scom.addPilot( pilots, sphalanx )
   elseif r < 0.8 then
      scom.addPilot( pilots, srhino )
   elseif r < 0.9 then
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
   pilots.__stealth = (rnd.rnd() < 0.7)
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sancestor )
      spir.add_hyena( pilots )
   elseif r < 0.5 then
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sancestor )
      scom.addPilot( pilots, sshark )
      spir.add_hyena( pilots )
   elseif r < 0.7 then
      scom.addPilot( pilots, srhino )
      scom.addPilot( pilots, sphalanx )
      scom.addPilot( pilots, sshark )
   elseif r < 0.85 then
      scom.addPilot( pilots, sadmonsher )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, sshark )
      spir.add_hyena( pilots )
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
   pilots.__stealth = (rnd.rnd() < 0.5)
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
function spir.create ( fpirate, max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spir.spawn_patrol  ] = 100
   weights[ spir.spawn_loner   ] = 100
   weights[ spir.spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spir.spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fpirate, weights, max )
end

return spir
