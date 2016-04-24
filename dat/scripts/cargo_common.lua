include "dat/scripts/jumpdist.lua"
include "dat/scripts/nextjump.lua"

-- Find an inhabited planet 0-3 jumps away.
function cargo_selectMissionDistance ()
   local seed = rnd.rnd()

   -- 70% chance of 0-3 jump distance
   if seed < 0.7 then
      seed = rnd.rnd()
      if seed < 0.30 then missdist = 0
      -- I assume this is supposed to be 50 and not 2 times 60
      elseif seed < 0.50 then missdist = 1
      elseif seed < 0.60 then missdist = 2
      else missdist = 3
      end
   else
      missdist = rnd.rnd(4, 6)
   end

   return missdist
end

-- Build a set of target planets
function cargo_selectPlanets(missdist, routepos)
   local planets = {}
   getsysatdistance(system.cur(), missdist, missdist,
      function(s)
         for i, v in ipairs(s:planets()) do
            if v:services()["inhabited"] and v ~= planet.cur() and v:class() ~= 0 and
                  not (s==system.cur() and ( vec2.dist( v:pos(), routepos ) < 2500 ) ) and
                  v:canLand() and cargoValidDest( v ) then
               planets[#planets + 1] = {v, s}
            end
         end
         return true
      end)

   return planets   
end

-- We have a destination, now we need to calculate how far away it is by simulating the journey there.
-- Assume shortest route with no interruptions.
-- This is used to calculate the reward.
function cargo_calculateDistance(routesys, routepos, destsys, destplanet)
   local traveldist = 0

   jumps = routesys:jumpPath( destsys )
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
   origin_p, origin_s = planet.cur()
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
   local destplanet = planets[index][1]
   local destsys   = planets[index][2]
   
   -- We have a destination, now we need to calculate how far away it is by simulating the journey there.
   -- Assume shortest route with no interruptions.
   -- This is used to calculate the reward.

   local numjumps   = origin_s:jumpDist(destsys)
   local traveldist = cargo_calculateDistance(routesys, routepos, destsys, destplanet)
   
   
   --Determine amount of piracy along the route
   local jumps = system.jumpPath( system.cur(), destsys:name() )
   local risk = system.cur():presences()["Pirate"]
   if risk == nil then risk = 0 end
   if jumps then
      for k, v in ipairs(jumps) do
         local travelrisk = v:system():presences()["Pirate"]
         if travelrisk == nil then
            travelrisk = 0
         end
         local risk = risk+travelrisk
      end
   end
	local avgrisk = risk/(numjumps + 1)
   
   -- We now know where. But we don't know what yet. Randomly choose a commodity type.
   -- TODO: I'm using the standard cargo types for now, but this should be changed to custom cargo once local-defined commodities are implemented.
   local cargoes = difference(planet.cur():commoditiesSold(),destplanet:commoditiesSold())
   if #cargoes == 0 then
      return
   end
   local cargo = cargoes[rnd.rnd(1,#cargoes)]:name()

   -- Return lots of stuff
   return destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier
end


-- Construct the cargo mission description text
function buildCargoMissionDescription( priority, amount, ctype, destplanet, destsys )
   str = "Shipment to %s"
   if priority ~= nil then
      str = priority .. " transport to %s"
   end
   if system.cur() ~= destsys then
      str = string.format( "%s in %s", str, destsys:name() )
   end
   return string.format( "%s (%s tonnes)", str:format( destplanet:name()), amount )
end


-- Calculates the minimum possible time taken for the player to reach a destination.
function cargoGetTransit( timelimit, numjumps, traveldist )
   local pstats   = player.pilot():stats()
   local stuperpx = 1 / pstats.speed_max * 30
   local arrivalt = time.get() + time.create(0, 0, traveldist * stuperpx +
         numjumps * pstats.jump_delay + 10180 + 240 * numjumps)
   return arrivalt
end

function cargoValidDest( targetplanet )
   -- The blacklist are factions which cannot be delivered to by factions other than themselves, i.e. the Thurion and Proteron.
   local blacklist = {
                     faction.get("Proteron"),
                     faction.get("Thurion"),
                     }
   for i,f in ipairs( blacklist ) do
      if planet.cur():faction() == blacklist[i] and targetplanet:faction() ~= blacklist[i] then
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
    for k,v in pairs(a) do r[k] = v; ai[v]=true end
    for k,v in pairs(b) do 
        if ai[v]~=nil then   r[k] = nil   end
    end
    return r
end

