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
   local pilots = { __doscans = true }
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sreaver )
   elseif r < 0.8 then
      scom.addPilot( pilots, smarauder )
      scom.addPilot( pilots, sbrigand )
   else
      scom.addPilot( pilots, snyx )
   end

   return pilots
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = {}
   if rnd.rnd() < 0.5 then
      pilots.__doscans = true
   end
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sodium )
      scom.addPilot( pilots, smarauder )
      scom.addPilot( pilots, sbrigand )
   elseif r < 0.8 then
      scom.addPilot( pilots, sodium )
      scom.addPilot( pilots, sreaver )
   else
      scom.addPilot( pilots, snyx )
      scom.addPilot( pilots, sreaver )
      scom.addPilot( pilots, sbrigand )
   end

   return pilots
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()

   -- Generate the capship
   if r < 0.1 then
      scom.addPilot( pilots, scopia )
   elseif r < 0.7 then
      scom.addPilot( pilots, sira )
   else
      scom.addPilot( pilots, sarx )
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, sreaver )
      scom.addPilot( pilots, smarauder )
      scom.addPilot( pilots, sbrigand )
   elseif r < 0.8 then
      scom.addPilot( pilots, sodium )
      scom.addPilot( pilots, sreaver )
   else
      scom.addPilot( pilots, snyx )
      scom.addPilot( pilots, sreaver )
   end

   return pilots
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
