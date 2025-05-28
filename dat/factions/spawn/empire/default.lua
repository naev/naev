local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local sshark      = ship.get("Empire Shark")
local shawking    = ship.get("Empire Hawking")
local speacemaker = ship.get("Empire Peacemaker")
local srainmaker  = ship.get("Empire Rainmaker")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.5, var.lancelot },
      { w=0.8, var.lancelot, sshark },
      { var.empire_pacifier },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.5, var.empire_admonisher, var.lancelot },
      { w=0.8, var.empire_admonisher, var.lancelot, sshark },
      { var.empire_pacifier, var.lancelot, sshark },
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
      { w=0.5, var.lancelot, var.lancelot, sshark },
      { w=0.8, var.empire_admonisher, var.lancelot },
      { var.empire_pacifier, var.lancelot },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
