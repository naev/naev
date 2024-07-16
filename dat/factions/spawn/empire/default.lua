local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Empire Lancelot")
local sshark      = ship.get("Empire Shark")
local sadmonisher = ship.get("Empire Admonisher")
local spacifier   = ship.get("Empire Pacifier")
local shawking    = ship.get("Empire Hawking")
local speacemaker = ship.get("Empire Peacemaker")
local srainmaker  = ship.get("Empire Rainmaker")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.5, slancelot },
      { w=0.8, slancelot, sshark },
      { spacifier },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.5, sadmonisher, slancelot },
      { w=0.8, sadmonisher, slancelot, sshark },
      { spacifier, slancelot, sshark },
   } )
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.1, srainmaker },
      { w=0.7, shawking },
      { speacemaker },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, slancelot, slancelot, sshark },
      { w=0.8, sadmonisher, slancelot },
      { spacifier, slancelot },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
