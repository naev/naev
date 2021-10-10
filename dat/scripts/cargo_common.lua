local lmisn = require "lmisn"
local pir = require 'common.pirate'
local fmt = require "format"

-- Don't use hidden jumps by default; set this to true to use hidden jumps.
cargo_use_hidden = false

-- By default, only generate if commodities available. Set to true to always generate.
cargo_always_available = false

-- Find an inhabited planet 0-3 jumps away.
function cargo_selectMissionDistance ()
   local seed = rnd.rnd()

   -- 70% chance of 0-3 jump distance
   if seed < 0.7 then
      missdist = rnd.rnd(0, 3)
   else
      missdist = rnd.rnd(4, 6)
   end

   return missdist
end

-- Build a set of target planets
function cargo_selectPlanets(missdist, routepos)
   local pcur = planet.cur()
   return lmisn.getPlanetAtDistance( system.cur(), missdist, missdist, "Independent", false, function ( p )
         if p ~= pcur
            and not (p:system() == system.cur() and (vec2.dist( p:pos(), routepos) < 2500))
            and p:canLand() and cargoValidDest( p ) then
               return true
         end
         return false
         end,
      nil, cargo_use_hidden)
end

-- We have a destination, now we need to calculate how far away it is by simulating the journey there.
-- Assume shortest route with no interruptions.
-- This is used to calculate the reward.
function cargo_calculateDistance(routesys, routepos, destsys, destplanet)
   local traveldist = 0

   local jumps = routesys:jumpPath( destsys, cargo_use_hidden )
   if jumps then
      for k, v in ipairs(jumps) do
         -- We're not in the destination system yet.
         -- So, get the next system on the route, and the distance between
         -- our entry point and the jump point to the next system.
         -- Then, set the exit jump point as the next entry point.
         local j, r = jump.get( v:system(), v:dest() )
         traveldist = traveldist + vec2.dist(routepos, j:pos())
         routepos = r:pos()
      end
   end

   -- We ARE in the destination system now, so route from the entry point to the destination planet.
   traveldist = traveldist + vec2.dist(routepos, destplanet:pos())

   return traveldist
end

function cargo_calculateRoute ()
   local origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- Select mission tier.
   local tier = rnd.rnd(0, 4)

   -- Farther distances have a lower chance of appearing.
   local missdist = cargo_selectMissionDistance()
   local planets = cargo_selectPlanets(missdist, routepos)
   if #planets == 0 then
      return
   end

   local index     = rnd.rnd(1, #planets)
   local destplanet= planets[index]
   local destsys   = destplanet:system()

   -- We have a destination, now we need to calculate how far away it is by simulating the journey there.
   -- Assume shortest route with no interruptions.
   -- This is used to calculate the reward.

   local numjumps   = origin_s:jumpDist(destsys, cargo_use_hidden)
   local traveldist = cargo_calculateDistance(routesys, routepos, destsys, destplanet)

   -- Guarding factions
   local _guards = {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Soromid"),
      faction.get("Sirius"),
      faction.get("Za'lek"),
      faction.get("Frontier"),
   }
   local function guard_presence( sys )
      local p = sys:presences()
      local total = 0
      for k,f in ipairs(_guards) do
         total = total + (p[f:nameRaw()] or 0)
      end
      return total
   end
   local function calc_risk( sys )
      local risk = pir.systemPresence( sys )
      local grisk = guard_presence( sys )
      return math.max( risk/10, risk - grisk/3 )
   end

   --Determine amount of piracy along the route
   local cursys = system.cur()
   local jumps = system.jumpPath( cursys, destsys )
   local risk = calc_risk( cursys )
   if risk == nil then risk = 0 end
   if jumps then
      for k, v in ipairs(jumps) do
         risk = risk + calc_risk( v:system() )
      end
   end
   risk = risk/(numjumps + 1)

   -- We now know where. But we don't know what yet. Randomly choose a commodity type.
   local cargo
   local cargoes = difference(planet.cur():commoditiesSold(),destplanet:commoditiesSold())
   if #cargoes == 0 then
      if cargo_always_available then
         cargo = nil
      else
         return
      end
   else
      cargo = cargoes[rnd.rnd(1,#cargoes)]:nameRaw()
   end

   -- Return lots of stuff
   return destplanet, destsys, numjumps, traveldist, cargo, risk, tier
end


-- Calculates the minimum possible time taken for the player to reach a destination.
function cargoGetTransit( numjumps, traveldist )
   local pstats   = player.pilot():stats()
   local stuperpx = 1 / pstats.speed_max * 30
   local arrivalt = time.get() + time.create(0, 0, traveldist * stuperpx +
         numjumps * pstats.jump_delay + 10180 + 240 * numjumps)
   return arrivalt
end

local _hidden_fact = {
   faction.get("FLF"),
   faction.get("Pirate"),
   faction.get("Marauder"),
   faction.get("Raven Clan"),
   faction.get("Dreamer Clan"),
   faction.get("Wild Ones"),
   faction.get("Black Lotus"),
   faction.get("Proteron"),
   faction.get("Thurion"),
}
function cargoValidDest( targetplanet )
   -- factions which cannot be delivered to by factions other than themselves
   local tfact = targetplanet:faction()
   for i, f in ipairs(_hidden_fact) do
      if tfact == f and planet.cur():faction() ~= f then
         return false
      end
   end

   -- Factions which cannot deliver to factions other than themselves
   local insular = {
      faction.get("Proteron"),
      faction.get("Thurion"),
   }
   for i, f in ipairs(insular) do
      if planet.cur():faction() == f and targetplanet:faction() ~= f then
         return false
      end
   end

   return true
end

--Determines the items in table a that are not in table b.
--Used to determine what cargo is sold at current planet but not at destination planet.
function difference(a, b)
   local ai = {}
   local r = {}
   for k,v in pairs(a) do
      r[k] = v
      ai[v]=true
   end
   for k,v in pairs(b) do
      if ai[v]~=nil then
         r[k] = nil
      end
   end
   return r
end

--[[
-- @brief Returns a block of mission-description text for the given cargo.
-- @tparam string misn_desc Translated title-level description, e.g. _("Cargo transport to %s in the %s system."):format(...).
-- @tparam string cargo Cargo type (raw name). May be nil.
-- @tparam number amount Cargo amount in tonnes. May be nil.
-- @param target Target planet for the delivery.
-- @param deadline Target delivery time. May be nil.
-- @param notes Any additional text the user should see on its own detail line, such as piracy risk. May be nil.
-- ]]
function cargo_setDesc( misn_desc, cargo, amount, target, deadline, notes )
   local t = { misn_desc, "" };
   if amount ~= nil then
      table.insert( t, _("#nCargo:#0 %s (%s)"):format( _(cargo), fmt.tonnes(amount) ) );
   elseif cargo ~= nil then
      table.insert( t, _("#nCargo:#0 %s"):format( _(cargo) ) );
   end

   local numjumps   = system.cur():jumpDist( target:system(), cargo_use_hidden )
   local dist = cargo_calculateDistance( system.cur(), planet.cur():pos(), target:system(), target )
   table.insert( t,
      gettext.ngettext( "#nJumps:#0 %d", "#nJumps:#0 %d", numjumps ):format( numjumps )
      .. "\n"
      .. gettext.ngettext("#nTravel distance:#0 %s", "#nTravel distance:#0 %s", dist):format( fmt.number(dist) ) )

   if notes ~= nil then
      table.insert( t, notes );
   end

   if deadline ~= nil then
      table.insert( t, _("#nTime limit:#0 %s"):format( tostring(deadline - time.get()) ) );
   end

   misn.setDesc( table.concat(t, "\n" ) );
end
