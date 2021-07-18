local lanes = {}

--[[
-- Gets the index to the nearest vertex to a position.
--]]
local function nearestVertex( vertices, pos )
   local n = nil
   local nd = math.huge
   for k,v in ipairs(vertices) do
      local d = v:dist(pos)
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
local function safelanesToGraph( lanes )
   local eps = 1e-5
   local vertices = {}
   local edges = {}
   for k,v in ipairs(lanes) do
      local v1, v2
      for i,p in ipairs(vertices) do
         if p:dist(v[1]) < eps then
            v1 = i
         end
         if p:dist(v[2]) < eps then
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

--[[
-- You run of the mill djikstra algorithm
--]]
local function djikstra( vertices, edges, source, target )
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
      u = Q[1]
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
-- Gets the closest point to a point p on a line segment a-b.
--]]
local function closestPointLine( a, b, p )
   local ap = p-a
   local ab = b-a
   local t  = ap:dot( ab ) / ab:dist2()
   t = math.max( 0, math.min( 1, t ) ) -- Clamp so it is on line segment
   return a + ab * t
end


--[[
-- Computes the route for the pilot to get to target.
--]]
function lanes.getRoute( target )
   -- We try to cache the lane graph per system
   local nc = naev.cache()
   if not nc.lanes then nc.lanes = {} end
   local ncl = nc.lanes
   local sc = system.cur()
   if ncl.system ~= sc then
      ncl.v, ncl.e = safelanesToGraph( safelanes.get() )
      ncl.system = sc
   end
   local lv, le = ncl.v, ncl.e

   -- Compute shortest path
   local aip = ai.pilot():pos()
   local sv = nearestVertex( lv, aip )
   local tv = nearestVertex( lv, target )
   local S = djikstra( lv, le, sv, tv )

   -- No path so just go straight
   if #S == 0 then
      return { target }
   end

   -- Correct the first point, as we might want to start in the middle of a
   -- line segment
   if #S > 1 then
      S[1] = closestPointLine( S[1], S[2], aip )
   end

   return S
end


return lanes
