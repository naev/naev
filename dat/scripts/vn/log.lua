local graphics = require 'love.graphics'

local log = {}

local _log, _header, _body, _colour

function log.reset ()
   _log = {}
end

function log.add( entry )
   table.insert( _log, 1, entry )
end

function log.open ()
   local lw, lh = graphics.getDimensions()
   local border = 100

   -- Build the tables
   -- TODO use vn.textbox_font
   log.font = graphics.newFont(16)
   local font = log.font
   local headerw = 280
   local bodyw = 800
   _header = {}
   _body = {}
   _colour = {}
   local th = 0
   for k,v in ipairs(_log) do
      local _maxw, headertext = font:getWrap( v.who, headerw )
      local _maxw, bodytext = font:getWrap( v.what, bodyw )

      local nlines = math.max( #headertext, #bodytext )
      for k=1,nlines do
         table.insert( _header, headertext[k] or "" )
         table.insert( _body,   bodytext[k] or "" )
         table.insert( _colour, v.colour )
      end
      th = th + nlines * font:getLineHeight()
   end

   -- Determine offset
   log.y = lh-border-th
   log.miny = log.y
   log.maxy = 100
end

function log.draw ()
   graphics.setColor( 0, 0, 0, 0.9 )
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
      y = y+lineh
      if y > 0 and y < lh then
         graphics.setColor( c[1], c[2], c[3], 1 )
         graphics.print( _header[k], font, headerx, y )
         graphics.print( _body[k],   font, bodyx,   y )
      end
   end
end

function log.update ()
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
end

function log.mousepressed( mx, my, button )
end

return log
