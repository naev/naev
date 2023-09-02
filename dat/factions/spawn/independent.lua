local scom = require "factions.spawn.lib.common"

local sschroedinger= ship.get("Schroedinger")
local sllama      = ship.get("Llama")
local sgawain     = ship.get("Gawain")
local skoala      = ship.get("Koala")
local shyena      = ship.get("Hyena")
local sshark      = ship.get("Shark")
local sancestor   = ship.get("Ancestor")
local slancelot   = ship.get("Lancelot")
local svendetta   = ship.get("Vendetta")
local sphalanx    = ship.get("Phalanx")
local sadmonisher = ship.get("Admonisher")
local sstarbridge = ship.get("Starbridge")
local svigilance  = ship.get("Vigilance")
local spacifier   = ship.get("Pacifier")
local skestrel    = ship.get("Kestrel")
local shawking    = ship.get("Hawking")
local sgoddard    = ship.get("Goddard")

-- Make pilot more visible
local function _advert( p )
   -- They want to be seen
   p:intrinsicSet( "ew_hide", 300 )
   p:intrinsicSet( "ew_signature", 300 )
end

local function spawn_advert ()
   local pilots = {}
   local civships = {
      sschroedinger,
      sllama,
      sgawain,
      shyena,
   }
   local shp = civships[ rnd.rnd(1, #civships) ]
   scom.addPilot( pilots, shp, {ai="advertiser", postprocess=_advert} )
   return pilots
end


-- @brief Spawns a small patrol fleet.
local function spawn_solitary_civilians ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.3 then
      scom.addPilot( pilots, sllama )
   elseif r < 0.55 then
      scom.addPilot( pilots, shyena )
   elseif r < 0.75 then
      scom.addPilot( pilots, sgawain )
   elseif r < 0.9 then
      scom.addPilot( pilots, sschroedinger )
   else
      scom.addPilot( pilots, skoala )
   end

   return pilots
end

local function spawn_bounty_hunter( shiplist )
   local pilots = {}
   local params = {name=_("Bounty Hunter"), ai="mercenary"}
   local shp    = shiplist[ rnd.rnd(1,#shiplist) ]
   scom.addPilot( pilots, shp, params )
   return pilots
end

local function spawn_bounty_hunter_sml ()
   return spawn_bounty_hunter{
      shyena,
      sshark,
      slancelot,
      svendetta,
      sancestor,
   }
end
local function spawn_bounty_hunter_med ()
   return spawn_bounty_hunter{
      sadmonisher,
      sphalanx,
      sstarbridge,
      svigilance,
      spacifier,
   }
end
local function spawn_bounty_hunter_lrg ()
   return spawn_bounty_hunter{
      skestrel,
      shawking,
      sgoddard,
   }
end

local findependent = faction.get("Independent")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Hostiles (namely pirates atm)
   local host = 0
   local total = 0
   local csys = system.cur()
   for f,v in pairs(csys:presences()) do
      if findependent:areEnemies(f) then
         host = host + v
      end
      total = total + v
   end
   local hostnorm = host / total

   -- Create weights for spawn table
   weights[ spawn_solitary_civilians ] = max
   weights[ spawn_bounty_hunter_sml  ] = math.min( 0.3*max, 50 )
   weights[ spawn_bounty_hunter_med  ] = math.min( 0.2*max, math.max(1, -150 + host ) )
   weights[ spawn_bounty_hunter_lrg  ] = math.min( 0.1*max, math.max(1, -300 + host ) )
   -- The more hostiles, the less advertisers
   -- The modifier should be 0.15 at 10% hostiles, 0.001 at 100% hostiles, and
   -- 1 at 0% hostiles
   weights[ spawn_advert  ] = 0.1 * max * math.exp(-hostnorm*5)

   return scom.init( findependent, weights, max )
end
