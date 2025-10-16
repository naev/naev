local lg = require 'love.graphics'
local centre = vec2.new( 0.5, 0.5 )

local function render( sp, x, y, z, dt )
   local d = sp:data()
   local sz = d.size * z
   d.timer = d.timer - dt

   local cleanup = false
   if not d.rot then
      for k,v in ipairs(d.r) do
         v.p = v.p + v.v * dt * 0.3
         local dc = v.p:dist( centre )
         if dc>1.1 then
            v.cleanup = true
            cleanup = true
         else
            v.a = math.min( 1, 5*(0.5 - dc) )
         end
      end
   else
      for k,v in ipairs(d.r) do
         v.p = v.p + v.v * dt * 0.3
         local vx, _y = v.p:get()
         if vx>1.1 then
            v.cleanup = true
            cleanup = true
         else
            v.a = math.min( 1, 5*(0.5 - math.abs(vx-0.5)) )
         end
      end
   end

   -- Rebuild table with valid elements only
   if cleanup then
      local nr = {}
      for k,v in ipairs(d.r) do
         if not v.cleanup then
            table.insert( nr, v )
         end
      end
      d.r = nr
   end

   -- Have to add a new one
   if d.timer <= 0 then
      local rz = 15 + 15*rnd.rnd()

      local cs = d.colspread
      local ncol = colour.new_hsv( d.col[1]+rnd.rnd()*cs*2-cs, d.col[2], d.col[3] )
      local col = table.pack( ncol:rgb(true) )
      col[4] = d.col[4]

      -- Initialize new
      if not d.rot then
         local r = rnd.angle()
         table.insert( d.r, {
            p = vec2.newP( 1, r ),
            v = vec2.newP( -1, r ),
            s = rz,
            a = 0,
            c = col,
         } )
      else
         table.insert( d.r, {
            p = vec2.new( 1, rnd.rnd()*(d.size-2*rz)/d.size+rz/d.size ),
            v = vec2.new( -1, 0 ),
            s = rz,
            a = 0,
            c = col,
         } )
      end

      d.timer = rnd.rnd()
   end

   -- Finally render the remaining
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
      lg.setColour( c )
      lg.rectangle( "fill", px*d.size, py*d.size, s, s )
   end

   lg.pop()
end

local function trail( pos, point, params )
   params = params or {}

   local size = params.size or 300

   local s = spfx.new( math.huge, nil, nil, nil, render, pos, nil, nil, size )
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
