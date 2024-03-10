local scom = require "factions.spawn.lib.common"

local sbrigand    = ship.get("Soromid Brigand")
local sreaver     = ship.get("Soromid Reaver")
local smarauder   = ship.get("Soromid Marauder")
local snyx        = ship.get("Soromid Nyx")
local sodium      = ship.get("Soromid Odium")
local sarx        = ship.get("Soromid Arx")
local sira        = ship.get("Soromid Ira")
local scopia      = ship.get("Soromid Copia")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans=true }, {
      { w=0.5, sreaver },
      { w=0.8, smarauder, sbrigand },
      { snyx },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans=(rnd.rnd() < 0.5) }, {
      { w=0.5, sodium, smarauder, sbrigand },
      { w=0.8, sodium, sreaver },
      { snyx, sreaver, sbrigand },
   } )
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.1, scopia },
      { w=0.7, sira },
      { sarx },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, sreaver, smarauder, sbrigand },
      { w=0.8, sodium, sreaver },
      { snyx, sreaver },
   } )
end

local fsoromid = faction.get("Soromid")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fsoromid, weights, max, {patrol=true} )
end
