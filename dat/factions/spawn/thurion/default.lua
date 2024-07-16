local scom = require "factions.spawn.lib.common"

local singenuity     = ship.get("Thurion Ingenuity")
local sperspicacity  = ship.get("Thurion Perspicacity")
local svirtuosity    = ship.get("Thurion Virtuosity")
local sapprehension  = ship.get("Thurion Apprehension")
local staciturnity   = ship.get("Thurion Taciturnity")
local scertitude     = ship.get("Thurion Certitude")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { w=0.3, singenuity },
      { w=0.6, singenuity, sperspicacity },
      { w=0.8, svirtuosity },
      { sapprehension },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {}, {
      { w=0.4, svirtuosity, singenuity, sperspicacity },
      { w=0.6, svirtuosity, singenuity },
      { w=0.8, staciturnity, sperspicacity, sperspicacity },
      { sapprehension, sperspicacity, sperspicacity },
   } )
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   -- Generate the capship
   scom.addPilot( pilots, scertitude )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, singenuity, singenuity, sperspicacity, sperspicacity },
      { w=0.8, svirtuosity, singenuity },
      { sapprehension, singenuity },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
