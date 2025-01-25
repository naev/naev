local scom = require "factions.spawn.lib.common"

local ssmall = ship.get("Lancelot")
local smedium = ship.get("Admonisher")
local slarge = ship.get("Hawking")

local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.6, ssmall },
      { ssmall, ssmall },
   } )
end

local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.6, smedium, ssmall },
      { smedium, ssmall, ssmall }
   } )
end

local function spawn_capship ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.3, slarge, ssmall, ssmall },
      { w=0.6, slarge, smedium, ssmall },
      { w=0.8, slarge, smedium, smedium },
      { slarge, smedium, smedium, ssmall, ssmall },
   } )
end

return function ( t, max )
   t.patrol      = { f = spawn_patrol, w = 100 }
   t.squad      = { f = spawn_squad,   w = max }
   t.capship   = { f = spawn_capship,   w = max }
end, 10
