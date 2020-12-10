--[[

Love2d API in Naev!!!!!
Meant to be loaded as a library to run Love2d stuff out of the box.

Example usage would be as follows:
"""
require 'love.lua'
require 'pong.lua'
love.start()
"""

--]]
love = {}

-- defaults
love.x = 0
love.y = 0
love.w = 800
love.h = 600
love.bgcol = colour.new( 0, 0, 0, 1 )
love.fgcol = colour.new( 1, 1, 1, 1 )

-- Internal function that connects to Naev
local function _update( dt )
   love.update(dt)
end
function love.update( dt ) end -- dummy

-- Internal function that connects to Naev
local function _mouse( x, y, mtype, button )
   x = x - love.x
   y = y - love.y
   print( string.format( "mouse: %.1f x %.1f, %s, %s", x, y, mtype, button ) )
   return true
end

--[[
-- Keyboard
--]]
love.keyboard = {}
love.keyboard._keystate = {}
-- Internal function that connects to Naev
local function _keyboard( pressed, key, mod )
   --print( string.format( "key: %s, %s, %s", pressed, key, mod ) )
   local k = string.lower( key )
   love.keyboard._keystate[ k ] = pressed
   if pressed then
      love.keypressed( k )
   end
   if key == "Q" then
      tk.customDone()
   end
   return true
end
function love.keypressed(key) end -- dummy
function love.keyboard.isDown( key )
   return (love.keyboard._keystate[ key ] == true)
end


--[[
-- Graphics
--]]
-- Internal function that connects to Naev
local function _draw( x, y, w, h )
   love.x = x
   love.y = y
   love.w = w
   love.h = h
   gfx.renderRect( x, y, w, h, love.bgcol )
   love.draw()
end
love.graphics = {}
local function _mode(m)
   if     m=="fill" then return false
   elseif m=="line" then return true
   else   error( string.format(_("Unknown fill mode '%s'"), mode ) )
   end
end
local function _xy( x, y, w, h )
   return love.x+x, love.y+(love.h-y-h)
end
function love.graphics.getWidth()
   return love.w
end
function love.graphics.getHeight()
   return love.h
end
local function _gcol( c )
   local r, g, b = c:rgb()
   local a = c:alpha()
   return r, g, b, a
end
local function _scol( r, g, b, a )
   if type(r)=="table" then
      a = r[4]
      b = r[3]
      g = r[2]
      r = r[1]
   end
   return colour.new( r, g, b, a or 1 )
end
function love.graphics.getBackgroundColor()
   return _gcol( self.bgcol )
end
function love.graphics.setBackgroundColor( red, green, blue, alpha )
   love.bgcol = _scol( red, green, blue, alpha )
end
function love.graphics.getColor()
   return _gcol( self.fgcol )
end
function love.graphics.setColor( red, green, blue, alpha )
   love.fgcol = _scol( red, green, blue, alpha )
end
function love.graphics.rectangle( mode, x, y, width, height )
   x,y = _xy(x,y,width,height)
   gfx.renderRect( x, y, width, height, love.fgcol, _mode(mode) )
end
function love.graphics.circle( mode, x, y, radius )
   x,y = _xy(x,y,0,0)
   gfx.renderCircle( x, y, radius, love.fgcol, _mode(mode) )   
end
function love.graphics.print( text, x, y  )
   x,y = _xy(x,y,limit,12)
   gfx.print( false, text, x, y, love.fgcol )
end
function love.graphics.printf( text, x, y, limit, align )
   x,y = _xy(x,y,limit,12)
   if align=="left" then
      gfx.print( false, text, x, y, love.fgcol, limit, false )
   elseif align=="center" then
      gfx.print( false, text, x, y, love.fgcol, limit, true )
   elseif align=="right" then
      local w = gfx.printDim( false, text, limit )
      local off = limit-w
      gfx.print( false, text, x+off, y, love.fgcol, w, false )
   end
end


--[[
-- Math
--]]
love.math = {}
function love.math.random( min, max )
   if min == nil then
      return rnd.rnd()
   elseif max == nil then
      return rnd.rnd( min-1 )+1
   else
      return rnd.rnd( min, max )
   end
end


--[[
-- Initialize
--]]
function love.start()
   -- Run set up function defined in Love2d spec
   love.load()

   -- Actually run in Naev
   tk.custom( "Test", love.w, love.h, _update, _draw, _keyboard, _mouse )
end

