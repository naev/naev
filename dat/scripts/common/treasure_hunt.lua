local lg = require "love.graphics"
local lmisn = require "lmisn"
local love_shaders = require "love_shaders"
local fmt = require "format"

local lib = {}

function lib.render( systems, jumps, w, h, target, names )
   local xmin, xmax, ymin, ymax = math.huge, -math.huge, math.huge, -math.huge
   local b = 30
   for k,s in ipairs(systems) do
      local x, y = s:pos():get()
      xmin = math.min( x-b, xmin )
      xmax = math.max( x+b, xmax )
      ymin = math.min( y-b, ymin )
      ymax = math.max( y+b, ymax )
   end
   local cx = (xmin+xmax)*0.5
   local cy = (ymin+ymax)*0.5
   local scale = 2/math.max( (xmax-xmin), (ymax-ymin) )

   -- Flip Y
   lg.push()
      lg.translate( 0, h )
      lg.scale( 1, -1 )

   local c = love_shaders.paper( w, h )
   lg.setCanvas( c )
   lg.setColour( 0, 0, 0, 0.3 )
   for k,j in ipairs(jumps) do
      local p1 = j:system():pos()
      local x1, y1 = p1:get()
      local p2 = j:dest():pos()
      local x2, y2 = p2:get()
      local len, ang = (p2-p1):polar()
      local x = (0.5+0.5*((x1+x2)*0.5-cx)*scale)*w
      local y = (0.5+0.5*((y1+y2)*0.5-cy)*scale)*h
      len = len*scale*w*0.5

      lg.push()
         lg.translate( x, y )
         lg.rotate( ang )
      lg.rectangle( "fill", -len*0.5, -2, len, 4 )
      lg.pop()
   end
   lg.setColour( 0.4, 0.4, 0.4, 1 )
   local r = 10
   for k,s in ipairs(systems) do
      local x, y = s:pos():get()
      lg.circle( "fill",
         w*(0.5+0.5*(x-cx)*scale),
         h*(0.5+0.5*(y-cy)*scale),
         r)
   end
   if target then
      lg.setColour( 0.8, 0.2, 0.3, 0.9 )
      local x, y = target:pos():get()
      lg.circle( "fill",
         w*(0.5+0.5*(x-cx)*scale),
         h*(0.5+0.5*(y-cy)*scale),
         r)
   end
   lg.setColour( 0.2, 0.2, 1, 1 )
   for k,s in ipairs(names) do
      local x, y = s:pos():get()
      lg.circle( "fill",
         w*(0.5+0.5*(x-cx)*scale),
         h*(0.5+0.5*(y-cy)*scale),
         r)
   end
   lg.pop()

   if #names > 0 then
      local font = lg.newFont( _("fonts/CoveredByYourGrace-Regular.ttf"), 24 )
      for k,s in ipairs(names) do
         local x, y = s:pos():get()
         local n = s:name()
         local maxw = font:getWrap( n, 1e6 )
         x = w*(0.5+0.5*(x-cx)*scale)
         y = h-h*(0.5+0.5*(y-cy)*scale)
         if x+r*1.5 > w-maxw then
            x = x - maxw - r*1.5
         else
            x = x + r*1.5
         end
         if y < 20 then
            y = y + 20
         elseif y > h-20 then
            y = y - 20
         end
         lg.push()
            lg.translate( x, y-r*0.75 )
            lg.rotate( rnd.rnd()-0.5 )
         lg.printf( s:name(), font, 0, 0, 1e6, "left" )
         lg.pop()
      end
   end
   lg.setCanvas()

   return c
end

local function makejumps( systems )
   local jumps = {}
   for k,s in ipairs(systems) do
      for i,j in ipairs(s:jumps()) do
         if inlist( systems, j:dest() ) and not inlist( jumps, j ) and not inlist( jumps, j:reverse() ) then
            table.insert( jumps, j )
         end
      end
   end
   return jumps
end

function lib.create_map_center( sys, w, h, n )
   n = n or 2
   local systems = lmisn.getSysAtDistance( sys, 0, n )
   return lib.render( systems, makejumps(systems), w, h, sys )
end

function lib.create_map_path( sstart, send, w, h, n )
   n = n or 1
   local systems = lmisn.getRoute( sstart, send )
   if n > 0 then
      for i=1,n do
         local newsys = tcopy(systems)
         for k,s in ipairs(systems) do
            for j,a in ipairs(s:adjacentSystems()) do
               if not inlist( newsys, a ) then
                  table.insert( newsys, a )
               end
            end
         end
         systems = newsys
      end
   end
   return lib.render( systems, makejumps(systems), w, h, send, {sstart} )
end

local function spob_check( p )
   if p:faction() then
      return false
   end
   local services = p:services()
   return not services["inhabited"] and services["land"]
end

function lib.create_treasure_hunt( center, maxdist )
   local sys = lmisn.getSysAtDistance( center, 0, maxdist, function( s )
      for k,p in ipairs(s:spobs()) do
         if spob_check(p) then
            return true
         end
      end
      return false
   end )
   if #sys <= 0 then return end
   sys = sys[rnd.rnd(1,#sys)]
   local spb = {}
   for k,p in ipairs(sys:spobs()) do
      if spob_check(p) then
         table.insert(spb,p)
      end
   end
   spb = spb[rnd.rnd(1,#spb)]
   local candidates = lmisn.getSysAtDistance( sys, 4, 5, function ( _s )
      return true
   end )
   if #candidates <= 0 then return end
   local start = candidates[rnd.rnd(1,#candidates)]
   local name = fmt.f(_("Near {sys}"),{sys=start})
   return {spb=spb, sys=sys, start=start, name=name}
end

local MISSIONNAME = "Treasure Hunt"
function lib.give_map( center, maxdist )
   maxdist = maxdist or 20
   local data = lib.create_treasure_hunt( center, maxdist )
   if not data then
      warn("Failed to give treasure map!")
      return false
   end
   if not player.misnActive( MISSIONNAME ) then
      naev.missionStart( MISSIONNAME )
   end
   naev.trigger( "treasure_hunt_add", data )
   return true
end

function lib.maps_owned()
   if not player.misnActive( MISSIONNAME ) then
      return 0
   end
   return naev.cache().treasure_maps
end

return lib
