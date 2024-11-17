local scom = require "factions.spawn.lib.common"

local sllama      = ship.get("Llama")
local skoala      = ship.get("Koala")
local smule       = ship.get("Mule")
local srhino      = ship.get("Rhino")
local szebra      = ship.get("Zebra")

local shyena      = ship.get("Hyena")
local sshark      = ship.get("Shark")
local sancestor   = ship.get("Ancestor")
local stristan    = ship.get("Tristan")
local slancelot   = ship.get("Lancelot")
local svendetta   = ship.get("Vendetta")
local sphalanx    = ship.get("Phalanx")
local sadmonisher = ship.get("Admonisher")
local sstarbridge = ship.get("Starbridge")
local svigilance  = ship.get("Vigilance")
local sbedivere   = ship.get("Bedivere")
local spacifier   = ship.get("Pacifier") -- codespell:ignore spacifier
local skestrel    = ship.get("Kestrel")
local shawking    = ship.get("Hawking")
local sgoddard    = ship.get("Goddard")

local semplancelot   = ship.get("Empire Lancelot")
local sempshark      = ship.get("Empire Shark")
local sempadmonisher = ship.get("Empire Admonisher")
local semppacifier   = ship.get("Empire Pacifier")
local semphawking    = ship.get("Empire Hawking")
local semppeacemaker = ship.get("Empire Peacemaker")
local semprainmaker  = ship.get("Empire Rainmaker")

local sdvvendetta   = ship.get("Dvaered Vendetta")
local sdvancestor   = ship.get("Dvaered Ancestor")
local sdvphalanx    = ship.get("Dvaered Phalanx")
local sdvvigilance  = ship.get("Dvaered Vigilance")
local sdvretribution= ship.get("Dvaered Retribution")
local sdvgoddard    = ship.get("Dvaered Goddard")
local sdvarsenal    = ship.get("Dvaered Arsenal")

local sbrigand    = ship.get("Soromid Brigand")
local sreaver     = ship.get("Soromid Reaver")
local smarauder   = ship.get("Soromid Marauder")
local snyx        = ship.get("Soromid Nyx")
local sodium      = ship.get("Soromid Odium")
local sarx        = ship.get("Soromid Arx")
local sira        = ship.get("Soromid Ira")
local scopia      = ship.get("Soromid Copia")

local seuler      = ship.get("Proteron Euler")
local sdalton     = ship.get("Proteron Dalton")
local shippocrates= ship.get("Proteron Hippocrates")
local sgauss      = ship.get("Proteron Gauss")
local spythagoras = ship.get("Proteron Pythagoras")
local sarchimedes = ship.get("Proteron Archimedes")
local swatson     = ship.get("Proteron Watson")

local spirhyena      = ship.get("Pirate Hyena")
local spirshark      = ship.get("Pirate Shark")
local spirvendetta   = ship.get("Pirate Vendetta")
local spirancestor   = ship.get("Pirate Ancestor")
local spirphalanx    = ship.get("Pirate Phalanx")
local spiradmonisher = ship.get("Pirate Admonisher")
local spirrhino      = ship.get("Pirate Rhino")
local spirstarbridge = ship.get("Pirate Starbridge")
local spirkestrel    = ship.get("Pirate Kestrel")

local function sample_one( tbl )
   local plts = {}
   scom.addPilot( plts, tbl[ rnd.rnd(1,#tbl) ] )
   return plts
end

local function spawn_trader ()
   return sample_one( {
      -- Generic
      sllama,
      skoala,
      smule,
      srhino,
      szebra,
   } )
end

local function spawn_small ()
   return sample_one( {
      -- Generic
      shyena,
      sshark,
      sancestor,
      slancelot,
      stristan,
      svendetta,
      -- Empire
      semplancelot,
      sempshark,
      -- Dvaered
      sdvvendetta,
      sdvancestor,
      -- Soromid
      sbrigand,
      sreaver,
      smarauder,
      -- Proteron
      seuler,
      sdalton,
      -- Pirate
      spirhyena,
      spirshark,
      spirvendetta,
      spirancestor,
   } )
end

local function spawn_medium ()
   return sample_one( {
      -- Generic
      sphalanx,
      sadmonisher,
      sstarbridge,
      svigilance,
      sbedivere,
      spacifier, -- codespell:ignore spacifier
      -- Empire
      sempadmonisher,
      semppacifier,
      -- Dvaered
      sdvphalanx,
      sdvvigilance,
      -- Soromid
      snyx,
      sodium,
      -- Proteron
      shippocrates,
      sgauss,
      -- Pirate
      spirphalanx,
      spiradmonisher,
      spirrhino,
      spirstarbridge,
   } )
end

local function spawn_large ()
   return sample_one( {
      -- Generic
      skestrel,
      shawking,
      sgoddard,
      -- Empire
      semphawking,
      semppeacemaker,
      semprainmaker,
      -- Dvaered
      sdvretribution,
      sdvarsenal,
      sdvgoddard,
      -- Soromid
      sarx,
      sira,
      scopia,
      -- Proteron
      spythagoras,
      sarchimedes,
      swatson,
      -- Pirate
      spirkestrel,
   } )
end

return function ( t, max )
   t.trader = { f = spawn_trader, w = 300-max }
   t.small  = { f = spawn_small,  w = math.max(1, max)  }
   t.medium = { f = spawn_medium, w = math.max(1, -80 + max) }
   t.large  = { f = spawn_large,  w = math.max(1, -300 + max) }
end, 10
