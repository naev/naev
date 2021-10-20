local graphics = require 'love.graphics'
local love_shaders = require "love_shaders"

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

function log.open ()
   local lw, lh = graphics.getDimensions()
   local border = log.border
   log.uparrow_alpha = 0
   log.downarrow_alpha = 0
   log.alpha = 0
   log.closing = false

   -- Build the tables
   -- TODO use vn.textbox_font
   log.font = graphics.newFont(16)
   local font = log.font
   local headerw = log.headerw
   local bodyw = log.bodyw
   _header = {}
   _body = {}
   _colour = {}
   local th = 0
   for id=#_log,1,-1 do
      local v = _log[id]
   end
   for id=#_log,1,-1 do
      local v = _log[id]
      local _maxw, headertext = font:getWrap( v.who, headerw )
      local _maxw, bodytext = font:getWrap( v.what, bodyw )

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
   log.y = lh-border-th
   log.miny = log.y
   log.maxy = log.border

   -- Compile shaders
   if not log.shader.arrow then
      log.shader.arrow = graphics.newShader( [[
#include "lib/sdf.glsl"
vec4 effect( vec4 colour, Image tex, vec2 pos, vec2 px )
{
   vec2 uv = pos*2.0-1.0;
   uv = vec2(-uv.y,-uv.x);

   float d1 = sdTriangleIsosceles( uv+vec2(0.0,-0.4), vec2(0.6,0.4) );
   float d2 = sdTriangleIsosceles( uv+vec2(0.0,0.2),  vec2(0.6,0.4) );
   float d3 = sdTriangleIsosceles( uv+vec2(0.0,0.8),  vec2(0.6,0.4) );

   float d = min(min(d1, d2), d3);

   d = abs(d)-0.01;

   colour.a *= step( 0.0, -d ) + pow( 1.0-d, 20.0 );
   return colour;
}
]], love_shaders.vertexcode)
   end
end

function log.draw ()
   graphics.setColor( 0, 0, 0, 0.9*log.alpha )
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
            graphics.setColor( c[1], c[2], c[3], log.alpha )
            graphics.print( _header[k], font, headerx, y )
            graphics.print( _body[k],   font, bodyx,   y )
         end
      end
   end

   x = log.border + log.headerw + log.bodyw + log.spacer
   if log.uparrow_alpha > 0 then
      graphics.setColor( 0, 1, 1, log.uparrow_alpha )
      graphics.setShader( log.shader.arrow )
      graphics.draw( love_shaders.img, x, 100, -math.pi/2, 60, 20 )
      graphics.setShader()
   end

   if log.downarrow_alpha > 0 then
      graphics.setColor( 0, 1, 1, log.downarrow_alpha )
      graphics.setShader( log.shader.arrow )
      graphics.draw( love_shaders.img, x, lh-100, math.pi/2, 60, 20 )
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

function log.mousepressed( mx, my, button )
end

return log
