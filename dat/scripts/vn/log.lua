local graphics = require 'love.graphics'
local sdf = require "vn.sdf"

local log = {
   border = 100,
   spacer = 10,
   headerw = 280,
   bodyw = 800,
   shader = {},
}

local _log, _header, _body, _colour

function log.reset ()
   _log = {
      { who="", what=_("[START]"), colour={1,1,1} },
   }
end

function log.add( entry )
   table.insert( _log, 1, entry )
end

function log.open( font )
   local _lw, lh = graphics.getDimensions()
   local border = log.border
   log.uparrow_alpha = 0
   log.downarrow_alpha = 0
   log.alpha = 0
   log.closing = false

   -- Build the tables
   log.font = font
   local headerw = log.headerw
   local bodyw = log.bodyw
   _header = {}
   _body = {}
   _colour = {}
   local th = 0
   for id=#_log,1,-1 do
      local v = _log[id]
      local _headw, headertext = font:getWrap( v.who, headerw )
      local _bodyw, bodytext = font:getWrap( v.what, bodyw )

      local nlines = math.max( #headertext, #bodytext )
      for k=1,nlines do
         table.insert( _header, headertext[k] or "" )
         table.insert( _body,   bodytext[k] or "" )
         table.insert( _colour, v.colour )
      end
      table.insert( _header, "_" )
      table.insert( _body, "_" )
      table.insert( _colour, "_" )
      th = th + nlines * font:getLineHeight() + log.spacer
   end

   -- Determine offset
   if th < lh then
      log.y    = (lh-th)/2
      log.miny = log.y
      log.maxy = log.y
   else
      log.y    = lh-border-th
      log.miny = log.y
      log.maxy = log.border
   end

   -- Compile shaders
   if not log.shader.arrow then
      log.shader.arrow = sdf.arrow
   end
end

function log.draw ()
   -- Drawn ontop of text so have to clear depth
   naev.gfx.clearDepth()

   graphics.setColour( 0, 0, 0, 0.9*log.alpha )
   local lw, lh = graphics.getDimensions()
   graphics.rectangle( "fill", 0, 0, lw, lh )

   local font = log.font
   local x = (lw-1080)/2
   local headerx = x
   local bodyx = x+200
   local lineh = font:getLineHeight()
   local y = log.y - lineh
   for k = 1,#_header do
      local c = _colour[k]
      if c == "_" then
         y = y+log.spacer
      else
         y = y+lineh
         if y > 0 and y < lh then
            graphics.setColour( c[1], c[2], c[3], log.alpha )
            graphics.print( _header[k], font, headerx, y )
            graphics.print( _body[k],   font, bodyx,   y )
         end
      end
   end

   x = log.border + log.headerw + log.bodyw + log.spacer
   if log.uparrow_alpha > 0 then
      graphics.setColour( 0, 1, 1, log.uparrow_alpha )
      graphics.setShader( log.shader.arrow )
      graphics.draw( sdf.img, x, 100, -math.pi/2, 60, 20 )
      graphics.setShader()
   end

   if log.downarrow_alpha > 0 then
      graphics.setColour( 0, 1, 1, log.downarrow_alpha )
      graphics.setShader( log.shader.arrow )
      graphics.draw( sdf.img, x, lh-100, math.pi/2, 60, 20 )
      graphics.setShader()
   end
end

function log.update( dt )
   dt = 5  * dt

   if log.y < log.maxy then
      log.uparrow_alpha = math.min( 1, log.uparrow_alpha+dt )
   else
      log.uparrow_alpha = math.max( 0, log.uparrow_alpha-dt )
   end

   if log.y > log.miny then
      log.downarrow_alpha = math.min( 1, log.downarrow_alpha+dt )
   else
      log.downarrow_alpha = math.max( 0, log.downarrow_alpha-dt )
   end

   if log.closing then
      log.alpha = log.alpha - dt
      if log.alpha <= 0 then
         return false
      end
   else
      log.alpha = math.min( 1, log.alpha + dt )
   end

   return true
end

function log.keypress( key )
   local lh = log.font:getLineHeight()
   if key=="up" then
      log.y = log.y + lh
   elseif key=="pageup" then
      log.y = log.y + 20*lh
   elseif key=="down" then
      log.y = log.y - lh
   elseif key=="pagedown" then
      log.y = log.y - 20*lh
   elseif key=="home" then
      log.y = log.maxy
   elseif key=="end" then
      log.y = log.miny
   end
   log.y = math.max( log.miny, math.min( log.maxy, log.y ) )

   if key=="tab" or key=="escape" or key=="space" or key=="enter" then
      log.closing = true
   end

   return true
end

function log.mousepressed( _mx, _my, _button )
end

return log
