local lanes = {}

--[[
-- Gets the index to the nearest vertex to a position.
--]]
local function nearestVertex( vertices, pos )
   local n = nil
   local nd = math.huge
   for k,v in ipairs(vertices) do
      local d = v:dist2(pos)
      if d < nd then
         nd = d
         n = k
      end
   end
   return n
end

--[[
-- Converts the safelane structure to a nice list of vertices and edges
--]]
local function safelanesToGraph( safelanes )
   local eps = 1e-5
   local vertices = {}
   local edges = {}
   for k,v in ipairs(safelanes) do
      local v1, v2
      for i,p in ipairs(vertices) do
         if p:dist2(v[1]) < eps then
            v1 = i
         end
         if p:dist2(v[2]) < eps then
            v2 = i
         end
      end
      if not v1 then
         local n = #vertices+1
         vertices[n] = v[1]
         v1 = n
      end
      if not v2 then
         local n = #vertices+1
         vertices[n] = v[2]
         v2 = n
      end
      table.insert( edges, {v1, v2} )
   end
   return vertices, edges
end

local function connected( vertices, edges, source )
   local L = {}
   for k,v in ipairs(vertices) do
      L[k] = 0
   end
   L[source] = 1

   local N = { source }
   while #N > 0 do
      for k,v in ipairs(N) do
         L[v] = 1
      end
      local NN = {}
      for k,v in ipairs(N) do
         for i,e in ipairs(edges) do
            if e[1] == v and L[e[2]] == 0 then
               table.insert( NN, e[2] )
            elseif e[2] == v and L[e[1]] == 0 then
               table.insert( NN, e[1] )
            end
         end
      end
      N = NN
   end

   local Q = {}
   for k,v in ipairs(L) do
      if v then
         table.insert( Q, vertices[k] )
      end
   end
   return Q
end

--[[
-- Your run of the mill Dijkstra algorithm. Currently dijkstra_full is used instead.
--]]
local function _dijkstra( vertices, edges, source, target )
   local Q = {}
   for k,v in ipairs(vertices) do
      local q = {
         v=k, -- Index of vertex
         d=math.huge, -- Gigantic initial distance
         p=nil, -- No previous node
      }
      table.insert( Q, q )
   end

   -- Build edges of qs
   local E = {}
   for k,e in ipairs(edges) do
      table.insert( E, {Q[e[1]], Q[e[2]]} )
   end

   -- Initialize source
   Q[source].d = 0

   -- Start iterating
   while #Q > 0 do
      -- Vertex with minimum distance
      table.sort( Q, function(a, b) return a.d < b.d end )
      local u = Q[1]
      table.remove( Q, 1 )

      -- Search done
      if u.v == target then
         -- Create the path by iterating backwards
         local S = {}
         while u do
            table.insert( S, vertices[u.v] )
            u = u.p
         end
         return S
      end

      -- Get neighbours
      local N = {}
      for k,e in ipairs(E) do
         if e[1] == u then
            table.insert( N, e[2] )
         elseif e[2] == u then
            table.insert( N, e[1] )
         end
      end

      -- Update distance
      for k,v in ipairs(N) do
         local alt = u.d + vertices[u.v]:dist( vertices[v.v] )
         if alt < v.d then
            v.d = alt
            v.p = u
         end
      end
   end

   -- No path found
   if target then
      return nil
   end
   -- Return the entire structure if no target
   return Q
end

--[[
-- Dijkstra but considering everything is fully connected with different
-- penalty for travelling through non-safe areas.
--]]
local function dijkstra_full( vertices, edges, source, target )
   local Q = {}
   local N = {}
   for k,v in ipairs(vertices) do
      local q = {
         v=k, -- Index of vertex
         d=math.huge, -- Gigantic initial distance
         p=nil, -- No previous node
      }
      table.insert( Q, q )
      table.insert( N, q )
   end

   -- Build penalty matrix
   local P = {}
   local n = #vertices
   for i=1,n do
      P[i] = {}
      for j=1,n do
         P[i][j] = 10 -- penalty
      end
   end
   for k,e in ipairs(edges) do
      P[e[1]][e[2]] = 1
      P[e[2]][e[1]] = 1
   end

   -- Initialize source
   Q[source].d = 0

   -- Start iterating
   while #Q > 0 do
      -- Vertex with minimum distance
      table.sort( Q, function(a, b) return a.d < b.d end )
      local u = Q[1]
      table.remove( Q, 1 )

      -- Search done
      if u.v == target then
         -- Create the path by iterating backwards
         local S = {}
         while u do
            table.insert( S, vertices[u.v] )
            u = u.p
         end
         return S
      end

      -- Fully connected, so all are neighbours
      for k,v in ipairs(N) do
         local p = P[u.v][k]
         local alt = u.d + p*vertices[u.v]:dist( vertices[v.v] )
         if alt < v.d then
            v.d = alt
            v.p = u
         end
      end
   end

   -- No path found
   if target then
      return nil
   end
   -- Return the entire structure if no target
   return Q
end


--[[
-- Gets the closest point to a point p on a line segment a-b.
--]]
local function closestPointLine( a, b, p )
   local ap = p-a
   local ab = b-a
   local t  = ap:dot( ab ) / ab:dist2()
   t = math.max( 0, math.min( 1, t ) ) -- Clamp so it is on line segment
   return a + ab * t
end

-- Caches lanes locally in pilot's memory
local function getCacheP( p )
   local mem = p:memory()
   if not mem.__lanes then
      local standing = (mem.lanes_useneutral and "non-hostile") or "friendly"
      mem.__lanes = lanes.get( p:faction(), standing, p:withPlayer() )
   end
   return mem.__lanes
end
-- Same thing for hostile lanes
local function getCachePH( p )
   local mem = p:memory()
   if not mem.__lanes_H then
      mem.__lanes_H = lanes.get( p:faction(), "hostile", p:withPlayer() )
   end
   return mem.__lanes_H
end

-- Clears the cache for persistent pilots and such
function lanes.clearCache( p )
   local mem = p:memory()
   mem.__lanes = nil
   mem.__lanes_H = nil
end


-- Same as safelanes.get but does caching
function lanes.get( f, standing, isplayer )
   -- We try to cache the lane graph per system
   local nc = naev.cache()
   if not nc.lanes then nc.lanes = {} end
   local ncl = nc.lanes
   local sc = system.cur()
   if ncl.system ~= sc then
      ncl.L = {}
   end
   local key = f:nameRaw()..standing
   if not ncl.L[key] then
      local L = {}
      L.lanes  = safelanes.get( f, standing, nil, isplayer )
      L.v, L.e = safelanesToGraph( L.lanes )
      ncl.L[key] = L
   end
   return ncl.L[key]
end


--[[
-- Gets distance and nearest point to safe lanes from a position
--]]
function lanes.getDistance( L, pos )
   local d, lpos = lanes.getDistance2( L, pos )
   return math.sqrt(d), lpos
end
function lanes.getDistanceP( p, pos )
   return lanes.getDistance( getCacheP(p), pos )
end
function lanes.getDistancePH( p, pos )
   return lanes.getDistance( getCachePH(p), pos )
end


--[[
-- Gets squared distance and nearest point to safe lanes from a position
--]]
function lanes.getDistance2( L, pos )
   local d = math.huge
   local lp = pos
   for k,v in ipairs(L.lanes) do
      local pp = closestPointLine( v[1], v[2], pos )
      local dp = pos:dist2( pp )
      if dp < d then
         d = dp
         lp = pp
      end
   end
   return d, lp
end
function lanes.getDistance2P( p, pos )
   return lanes.getDistance2( getCacheP(p), pos )
end
function lanes.getDistance2PH( p, pos )
   return lanes.getDistance2( getCachePH(p), pos )
end


function lanes.getPoint( L )
   local lv, le = L.v, L.e
   local elen = {}
   -- Compute total lane distance
   local td = 0
   for k,e in ipairs(le) do
      local d = lv[e[1]]:dist( lv[e[2]] )
      td = td + d
      table.insert( elen, d )
   end
   -- Choose a random pair based on the total distance
   local r = rnd.rnd()
   local raccum = 0
   for k,d in ipairs(elen) do
      local rd = d / td
      raccum = raccum + rd
      if r < raccum then
         local e = le[k]
         local a = (raccum-r) / rd
         return lv[e[1]] * a + lv[e[2]] * (1-a)
      end
   end
   return nil
end
function lanes.getPointP( p )
   return lanes.getPoint( getCacheP(p) )
end


--[[
-- Tries to get a point outside of the lanes, around a point at a radius rad.
-- We'll project the pos into radius if it is out of bounds.
--]]
function lanes.getNonPoint( L, pos, rad, margin, biasdir )
   local margin2 = margin*margin
   local srad2 = math.pow( system.cur():radius(), 2 )

   -- Make sure pos is in radius
   if pos:dist2() > srad2 then
      local _m,a = pos:polar()
      pos = vec2.newP( system.cur():radius(), a )
   end

   -- Just some brute force sampling at different scales
   local n = 18
   local inc = 2*math.pi / n
   local sign = 1
   if rnd.rnd() < 0.5 then sign = -1 end
   for s in ipairs{1.0, 0.5, 1.5, 2.0, 3.0, 5.0} do
      local a = biasdir or rnd.angle()
      local r = rad * s
      for i=1,n do
         local pp = pos + vec2.newP( r, a )
         a = a + i * inc * sign
         sign = sign * -1
         if pp:dist2() < srad2 then
            local d = lanes.getDistance2( L, pp )
            if d > margin2 then
               return pp
            end
         end
      end
   end
   return nil
end
function lanes.getNonPointP( p, pos, rad, margin, biasdir )
   local ews
   if not pos or not rad or not margin then
      ews = p:stats().ew_stealth
   end
   pos = pos or p:pos()
   rad = rad or math.min( 2000, ews )
   margin = margin or ews

   local L = getCacheP( p )
   return lanes.getNonPoint( L, pos, rad, margin, biasdir )
end

--[[
-- Gets a random point of interest
--]]
function lanes.getPointInterest( L, pos )
   local lv, le = L.v, L.e

   -- Case nothing of interest we just return a random position like in the old days
   -- TODO do something smarter here
   if #lv == 0 then
      return vec2.newP( rnd.rnd() * system.cur():radius(), rnd.angle() )
   end

   -- Get the connected components
   local sv = nearestVertex( lv, pos )
   local S = connected( lv, le, sv )
   local Sfar = {}
   for k,v in ipairs(S) do
      if pos:dist2(v) > 1000*1000 then -- TODO better threshold
         table.insert( Sfar, v )
      end
   end

   -- No far points, this shouldn't happen, but return random point in this case
   if #Sfar == 0 then
      return vec2.newP( rnd.rnd() * system.cur():radius(), rnd.angle() )
   end

   -- Random far away point
   return Sfar[ rnd.rnd(1, #Sfar) ]
end
function lanes.getPointInterestP( p, pos )
   pos = pos or p:pos()
   local L = getCacheP(p)
   return lanes.getPointInterest( L, pos )
end


--[[
-- Computes the route for the pilot to get to target.
--]]
function lanes.getRoute( L, target, pos )
   local lv, le = L.v, L.e

   -- Case no lanes in the system
   if #lv == 0 then
      return { target }
   end

   -- Compute shortest path
   local sv = nearestVertex( lv, pos )
   local tv = nearestVertex( lv, target )
   local S = dijkstra_full( lv, le, tv, sv )

   -- No path or target is closer so just go straight
   if #S==0 or pos:dist2( S[1] ) > pos:dist2(target) then
      return { target }
   end

   -- Add the final point if necessary (it is only approximated due to
   -- djistra)
   if S[#S]:dist2( target ) > 1e-5 then
      table.insert( S, target )
   end

   -- Correct the first point, as we might want to start in the middle of a
   -- line segment
   if #S > 1 then
      S[1] = closestPointLine( S[1], S[2], pos )
   end

   return S
end
function lanes.getRouteP( p, target, pos )
   pos = pos or p:pos()
   local L = getCacheP(p)
   return lanes.getRoute( L, target, pos )
end

return lanes
