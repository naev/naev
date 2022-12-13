local lg = require 'love.graphics'
local center = vec2.new( 0.5, 0.5 )

local function update( sp, dt )
   local d = sp:data()
   d.timer = d.timer - dt

   if not d.rot then
      for k,v in ipairs(d.r) do
         v.p = v.p + v.v * dt * 0.3
         v.a = math.min( 1, 5*(0.5 - v.p:dist( center ) ) )
      end
   else
      for k,v in ipairs(d.r) do
         v.p = v.p + v.v * dt * 0.3
         local x, _y = v.p:get()
         v.a = math.min( 1, 5*(0.5 - math.abs(x-0.5)) )
      end
   end

   if d.timer <= 0 then
      local sz = 15 + 15*rnd.rnd()

      local ncol = naev.colour.new()
      local cs = d.colspread
      ncol:setHSV( d.col[1]+rnd.rnd()*cs*2 - cs, d.col[2], d.col[3] )
      local col = table.pack( ncol:rgb(true) )
      col[4] = d.col[4]

      -- Initialize new
      if not d.rot then
         local r = rnd.angle()
         table.insert( d.r, {
            p = vec2.newP( 1, r ),
            v = vec2.newP( -1, r ),
            s = sz,
            a = 0,
				c = col,
         } )
      else
         table.insert( d.r, {
            p = vec2.new( 1, rnd.rnd()*(d.size-2*sz)/d.size+sz/d.size ),
            v = vec2.new( -1, 0 ),
            s = sz,
            a = 0,
				c = col,
         } )
      end

      d.timer = rnd.rnd()
   end
end

local function render( sp, x, y, z )
   local d = sp:data()
   local sz = d.size * z

   lg.push()
   lg.translate( x, y )
   if d.rot then
      lg.rotate( d.rot )
   end
   lg.translate( -sz*0.5, -sz*0.5 )
   lg.scale( z )

   for k,v in ipairs(d.r) do
      local px, py = v.p:get()
      local c = { v.c[1], v.c[2], v.c[3], v.c[4]*v.a }
      local s = v.s
      lg.setColor( c )
      lg.rectangle( "fill", px*d.size, py*d.size, s, s )
   end

   lg.pop()
end

local function trail( pos, point, params )
   params = params or {}

   local size = params.size or 300

   local s = spfx.new( math.huge, update, nil, nil, render, pos, nil, nil, size )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.col    = params.col or {0, 1.0, 0.7, 0.5} -- in HSV
	d.colspread = params.colspread or 50
   if point then
      local _m, dir = ((point-pos) * vec2.new(-1, 1)):polar()
      d.rot = dir
   end
   d.r = {}

   return s
end

return trail
