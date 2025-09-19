local ccomm = require  "common.comm"
local fmt = require "format"

local fct = faction.get("Independent")

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local function spawn_needs_refuel ()
   local scur = system.cur()
   if scur:tags().restricted then
      return false
   end
   local _nebu, vol = scur:nebula()
   if vol > 0 then
      return false
   end
   -- Ignore systems that have refuelling services (probably could check more in detail, but meh)
   for k,s in ipairs(scur:spobs()) do
      if s:services().refuel then
         return false
      end
   end
   -- Must be near a place with independent presence
   for k,s in ipairs(scur:adjacentSystems()) do
      if s:presence( fct ) > 0 then
         return true
      end
   end
   return false
end

return function ()
   -- Remote system
   if not spawn_needs_refuel() then
      return {}
   end

   return {
      spawn = function ()
         local ship = choose_one{ "Llama", "Schroedinger", "Koala", "Mule" }
         local plt = pilot.add(ship, "Independent", nil, nil, { ai="pers" } )

         local mem = plt:memory()
         mem.capturable = true
         mem.vulnerability = math.huge -- Less likely to be attacked
         mem.natural = true -- Can be captured and such
         mem.refuel_no = _([["Sorry, I'm also in need of fuel."]])

         local amount = plt:stats().fuel_consumption
         local reward = rnd.rnd(15e3, 25e3) * amount / 100

         local function needs_refuel( p )
            if player.evtActive("Refuel") then return false end
            if p:memory().refuelled then return false end
            return (not player.pilot():areEnemies(p) and p:fuel() < amount)
         end

         plt:credits( plt:credits() + reward )
         plt:setFuel( 0 )
         -- We use a function here and only try to hail once, delayed
         mem.ad = function( p )
            local pmem = p:memory()
            -- Make sure player is visible to hail
            local inr, nofuz = p:inrange( player.pilot() )
            if not pmem.fuel_hail and inr and nofuz then
               plt:hailPlayer()
               pmem.fuel_hail = true
            end
            if needs_refuel( p ) then
               return _("Looking for some fuel. Can anyone help?")
            end
         end
         mem.comm_greet = function ( p )
            if needs_refuel( p ) then
               return fmt.f(_([["I'm in somewhat of a pinch. Could you spare {amount} u of fuel? I'll pay {reward}."]]), {reward=fmt.credits(reward), amount=fmt.number(amount)})
            end
         end
         ccomm.customComm( plt, function ( p )
            if needs_refuel( p ) then
               return _("Offer to Refuel Them")
            end
            return nil
         end, function ( vn, _pvn, p )
            vn.func( function ()
               local nc = naev.cache()
               nc.__refuel = {
                  p = p,
                  reward = reward,
                  amount = amount,
               }
               p:memory().ad = nil -- Stop spamming
               naev.eventStart("Refuel")
            end )
            vn.done()
         end, "pers" )
      end
   }
end
