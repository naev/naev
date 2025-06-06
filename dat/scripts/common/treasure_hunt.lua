local lg = require "love.graphics"
local lf = require "love.filesystem"
local lmisn = require "lmisn"
local love_shaders = require "love_shaders"
local fmt = require "format"
local prng = require "prng"

local lib = {}

local BASEPATH = "gfx/misc/treasure_hunt/"
local gfx
local function loadgfx_dir( path )
   local imgs = {}
   for k,v in ipairs(lf.getDirectoryItems(BASEPATH..path)) do
      local img = lg.newImage( BASEPATH..path.."/"..v )
      if img then
         table.insert( imgs, img )
      end
   end
   return imgs
end

local function draw_object( obj, x, y, r, rng, rot )
   local o = obj[ rng:random(1,#obj) ]
   local s = r/50 -- 100px big images
   o:draw( x-r, y-r, rot, s, s )
end
local function draw_circle( x, y, r, rng )
   return draw_object( gfx.circle, x, y, r, rng, rng:random()*math.pi*2 )
end
local function draw_cross( x, y, r, rng )
   return draw_object( gfx.cross, x, y, r, rng, rng:random()*0.2 )
end
local function draw_triangle( x, y, r, rng )
   return draw_object( gfx.triangle, x, y, r, rng, rng:random()*0.2 )
end
local function draw_line( x1, y1, x2, y2, r, rng )
   local L = 20
   local dx, dy = x2-x1, y2-y1
   local len = math.sqrt(dx^2+dy^2)
   local ang = math.atan2( dy, dx )
   local N = math.max( 1, math.floor( (len-2*r) / L + 0.5 ) )
   local off = (len-N*L)*0.5
   local sx, sy = L*math.cos(ang), L*math.sin(ang)
   local x, y = x1+off*math.cos(ang)+0.5*sx, y1+off*math.sin(ang)+0.5*sy
   for i=1,N do
      draw_object( gfx.line, x, y, L*0.5, rng, ang )
      x = x+sx
      y = y+sy
   end
end

function lib.render( systems, jumps, w, h, target, names, rng )
   rng = rng or prng.new( rnd.rnd(1,2^30) )

   if gfx==nil then
      gfx = {
         line     = loadgfx_dir("line"),
         circle   = loadgfx_dir("circle"),
         cross    = loadgfx_dir("cross"),
         triangle = loadgfx_dir("triangle"),
      }
   end

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

   local function pos( x, y )
      return w*(0.5+0.5*(x-cx)*scale),
             h-h*(0.5+0.5*(y-cy)*scale)
   end

   -- Flip Y
   local c = love_shaders.paper( w, h, nil, rng )
   lg.setCanvas( c )
   lg.setColour( 0, 0, 0, 1 )
   local R = 20
   for k,j in ipairs(jumps) do
      local x1, y1 = pos( j:system():pos():get() )
      local x2, y2 = pos( j:dest():pos():get() )
      draw_line( x1, y1, x2, y2, R, rng )
   end
   for k,s in ipairs(systems) do
      if not inlist( names, s ) and target~=s then
         local x, y = pos(s:pos():get())
         draw_circle( x, y, R, rng )
      end
   end
   if target then
      lg.setColour( 0.8, 0.2, 0.3, 0.9 )
      local x, y = pos(target:pos():get())
      draw_cross( x, y, R, rng )
   end
   lg.setColour( 0.2, 0.2, 1, 1 )
   for k,s in ipairs(names) do
      local x, y = pos(s:pos():get())
      draw_triangle( x, y, R, rng )
   end

   if #names > 0 then
      local font = lg.newFont( _("fonts/CoveredByYourGrace-Regular.ttf"), 24 )
      for k,s in ipairs(names) do
         local x, y = pos( s:pos():get() )
         local n = s:name()
         local maxw = font:getWrap( n, 1e6 )
         if x+R*1.5 > w-maxw then
            x = x - maxw - R*1.5
         else
            x = x + R*1.5
         end
         if y < 20 then
            y = y + 20
         elseif y > h-20 then
            y = y - 20
         end
         lg.push()
            lg.translate( x, y-R*0.75 )
            lg.rotate( rng:random()-0.5 )
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

function lib.create_map_center( sys, w, h, n, rng )
   n = n or 2
   local systems = lmisn.getSysAtDistance( sys, 0, n )
   return lib.render( systems, makejumps(systems), w, h, sys, nil, rng )
end

function lib.create_map_path( sstart, sgoal, w, h, n, rng )
   n = n or 1
   local systems = lmisn.getRoute( sstart, sgoal )
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
   return lib.render( systems, makejumps(systems), w, h, sgoal, {sstart}, rng )
end

local function spob_check( p )
   if p:faction() then
      return false
   end
   local services = p:services()
   return (not services["inhabited"]) and services["land"]
end

function lib.create_map( data, w, h )
   local rng = prng.new( data.seed )
   return lib.create_map_path( data.start, data.goal, w, h, data.n, rng )
end

function lib.create_treasure_hunt( center, maxdist, length )
   length = length or rnd.rnd(4,5)
   maxdist = maxdist or 20
   local goallst = lmisn.getSysAtDistance( center, 0, maxdist, function( s )
      for k,p in ipairs(s:spobs()) do
         if spob_check(p) then
            return true
         end
      end
      return false
   end )
   if #goallst <= 0 then return end
   -- Try to see if we can find a pair for any of the targets
   goallst = rnd.permutation(goallst)
   for i,goal in ipairs(goallst) do
      local spb = {}
      for k,p in ipairs(goal:spobs()) do
         if spob_check(p) then
            table.insert(spb,p)
         end
      end
      spb = spb[rnd.rnd(1,#spb)] -- Should exist as we checked when getting goal
      local candidates = lmisn.getSysAtDistance( goal, length, length, function ( _s )
         -- TODO maybe add some criteria here to pick a target?
         return true
      end )
      if #candidates > 0 then
         local start = candidates[rnd.rnd(1,#candidates)]
         local name = fmt.f(_("Near {sys}"),{sys=start})
         return {
            spb=spb,
            goal=goal,
            start=start,
            name=name,
            seed=rnd.rnd(1,2^30),
         }
      end
   end
end

local MISSIONNAME = "Treasure Hunt"
function lib.give_map( center, maxdist )
   local data = lib.create_treasure_hunt( center, maxdist )
   if not data then
      warn("Failed to give treasure map!")
      return false
   end
   lib.give_map_from( data )
   return true
end

function lib.give_map_from( data )
   if not player.misnActive( MISSIONNAME ) then
      naev.missionStart( MISSIONNAME )
   end
   naev.trigger( "treasure_hunt_add", data )
end

function lib.maps_owned()
   if not player.misnActive( MISSIONNAME ) then
      return 0
   end
   return naev.cache().treasure_maps
end

return lib
