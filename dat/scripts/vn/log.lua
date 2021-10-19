local graphics = require 'love.graphics'

local log = {}

local _log, _header, _body, _colour

function log.reset ()
   _log = {}
end

function log.add( entry )
   table.insert( _log, entry )
end

function log.open ()
   -- TODO use vn.textbox_font
   log.font = graphics.newFont(16)
   local font = log.font
   local headerw = 200
   local bodyw = 880
   _header = {}
   _body = {}
   _colour = {}
   for k,v in ipairs(_log) do
      local _maxw, headertext = font:getWrap( v.who, headerw )
      local _maxw, bodytext = font:getWrap( v.what, bodyw )

      local nlines = math.max( #headertext, #bodytext )
      for k=1,nlines do
         table.insert( _header, headertext[k] or "" )
         table.insert( _body,   bodytext[k] or "" )
         table.insert( _colour, v.colour )
      end
   end
end

function log.draw ()
   graphics.setColor( 0, 0, 0, 0.8 )
   local lw, lh = graphics.getDimensions()
   graphics.rectangle( "fill", 0, 0, lw, lh )

   local font = log.font
   local x = (lw-1080)/2
   local headerx = x
   local bodyx = x+200
   local y = 100
   local lh = font:getLineHeight()
   for k,v in ipairs(_header) do
      local c = _colour[k]
      graphics.setColor( c[1], c[2], c[3], 1 )
      graphics.print( _header[k], font, headerx, y )
      graphics.print( _body[k],   font, bodyx,   y )
      y = y+lh
   end
end

function log.update ()
end

function log.keypress( key )
   if key=="up" or key=="pageup" then
   elseif key=="down" or key=="pagedown" then
   elseif key=="home" then
   elseif key=="end" then
   end
end

function log.mousepressed( mx, my, button )
end

return log
