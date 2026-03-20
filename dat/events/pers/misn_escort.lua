local ccomm = require  "common.comm"
local fmt = require "format"
local pir = require "common.pirate"
local lmisn = require "lmisn"

local FCT = faction.get("Traders Society")

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local function filterspob( spbs, ssys )
   local good = {}
   for k,v in ipairs(spbs) do
      if inlist( ssys, v:system() ) then
         table.insert( good, v )
      end
   end
   if #good > 0 then
      return choose_one( good )
   end
end

local function spawn_escort ()
   local scur = system.cur()
   if scur:tags().restricted then
      return false
   end
   local _nebu, vol = scur:nebula()
   if vol > 0 then
      return false
   end
   if pir.systemPresence( scur ) <= 0 then
      return false
   end
   if scur:presence( FCT ) <= 0 then
      return false
   end
   if player.outfitNum("Mercenary Licence") <= 0 then
      return false
   end

   -- Must not be active already
   if player.misnActive("Escort Pers") then
      return false
   end

   -- Look for good targets on the way to where the player is going ideally
   local jumps = player.jumps()
   if jumps < 2 then
      return false
   end
   local spbs = lmisn.getSpobAtDistance( nil, 1, math.min(3,jumps), FCT, false, function( spb )
      return pir.systemPresence( spb:system() ) > 0
end )
   if #spbs <= 0 then
      return false
   end
   -- Look at current route
   local markers = player.autonavRoute()
   local good = filterspob( spbs, markers )
   if good then return good end
   -- Test systems if necessary
   markers = {} -- No need to test route anymore
   for k,m in ipairs(player.missions()) do
      for i,mrk in ipairs(m.markers) do
         local sys
         if mrk.system then
            sys = mrk:system()
         else
            sys = mrk
         end
         if sys:jumpDist() < 5 then
            table.insert( markers, mrk )
         end
      end
   end
   if #markers <= 0 then
      return choose_one( spbs )
   end
   return filterspob( spbs, markers )
end

return function ()
   -- Remote system
   local dest = spawn_escort()
   if not dest then
      return {}
   end

   local difficulty  = rnd.rnd(1,5)
   local jumps       = dest:system():jumpDist()
   local cargo       = choose_one{
      commodity.get("Luxury Goods"),
      commodity.get("Medicine"),
      commodity.get("Gold"),
      commodity.get("Neodymium"),
      commodity.get("Yttrium"),
      commodity.get("Rhodium"),
      commodity.get("Platinum"),
   }
   local reward      = 50e3 + jumps * (7+difficulty) * cargo:price() * (1+0.05*rnd.twosigma())
   local name = choose_one{
      _("S.T.S. Yucatan"),
      _("S.T.S. Goldilocks"),
      _("S.T.S. Deimos"),
      _("S.T.S. Pilgrim"),
      _("S.T.S. Calypso"),
      _("S.T.S. Opal Star"),
      _("S.T.S. Kon Tiki"),
      _("S.T.S. Queen Mary"),
      _("S.T.S. Explorer of the Stars"),
      _("S.T.S. Victoria"),
      _("S.T.S. Halley's Comet"),
      _("S.T.S. Siren"),
      _("S.T.S. Tranquility"),
      _("S.T.S. Starry Night"),
      _("S.T.S. Little Rascal"),
      _("S.T.S. Tortoise"),
      _("S.T.S. Independence"),
      _("S.T.S. Sky Voyager"),
      _("S.T.S. Kingfisher"),
      _("S.T.S. New Beginning"),
      _("S.T.S. Ark Royal"),
      _("S.T.S. Nostradamus"),
      _("S.T.S. Event Horizon"),
      _("S.T.S. Determination"),
      _("S.T.S. Space Bounty"),
      _("S.T.S. Normandy"),
      _("S.T.S. Shooting Star"),
      _("S.T.S. Aurora"),
   }

   return {
      spawn = function ()
         local ship = choose_one{ "Quicksilver", "Koala", "Mule" }
         local plt = pilot.add(ship, FCT, nil, name, { ai="pers_runaway" } )

         local mem = plt:memory()
         mem.capturable = true
         mem.vulnerability = math.huge -- Less likely to be attacked
         mem.natural = true -- Can be captured and such
         plt:cargoRm( "all" )
         plt:cargoAdd( cargo, plt:cargoFree() )

         -- We use a function here and only try to hail once, delayed
         mem.ad = function( p )
            local pmem = p:memory()
            -- Make sure player is visible to hail
            local inr, nofuz = p:inrange( player.pilot() )
            if not pmem.pers_hail and inr and nofuz then
               plt:hailPlayer()
               pmem.pers_hail = true
            end
            return fmt.f(_("Looking for some protection to {dest} ({sys} system). Hail me if interested."), {
               dest  = dest,
               sys   = dest:system(),
            })
         end
         mem.comm_greet = function ( _p )
            return fmt.f(_([["I need some protection to get to {dest} in the {sys} system. I can pay {reward} if you are willing."]]), {
               dest  = dest,
               sys   = dest:system(),
               reward = fmt.credits(reward),
            })
         end
         ccomm.customComm( plt, function ( _p )
            return fmt.f(_("Escort them to {dest} ({sys} system)"), {
               dest  = dest,
               sys   = dest:system(),
            } )
         end, function ( vn, pvn, p )
            vn.func( function ()
               local nc = naev.cache()
               nc.__pers_escort = {
                  p        = p,
                  dest     = dest,
                  reward   = reward,
                  difficulty = difficulty,
                  cargo    = cargo,
               }
               p:memory().ad = nil -- Stop spamming
               naev.missionStart("Escort Pers")
            end )
            pvn(_([["Thanks! Keep an eye out for Marauders. I think they have been eyeing my cargo!"]]))
            vn.done()
         end, "pers" )
      end
   }
end
