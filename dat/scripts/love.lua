--[[

Love2d API in Naev!!!!!
Meant to be loaded as a library to run Love2d stuff out of the box.

Example usage would be as follows:
"""
require 'love'
require 'pong'
love.start()
"""

--]]
require 'class'

love = {}
love.basepath = ""
love._version_major = 11
love._version_minor = 1
love._version_patch = 3

-- Dummy user-defined functions
function love.conf(t) end -- dummy
function love.load() end --dummy

--[[
-- Base
--]]
function love.getVersion()
   return love._version_major, love._version_minor, love._version_patch
end


--[[
-- System
--]]
love.system = {}
function love.system.getOS()
   return "Naev"
end


--[[
-- Timer
--]]
love.timer = {}
love.timer._dt = 0
love.timer._adt = 0
love.timer._edt = 0
-- Internal function that connects to Naev
local function _update( dt )
   if love.keyboard._repeat then
      for k,v in pairs(love.keyboard._keystate) do
         if v then
            love.keypressed( k, k, true )
         end
      end
   end
   love.timer._edt = love.timer._edt + dt
   love.timer._dt = dt
   local alpha = 0.1
   love.timer._adt = alpha*dt + (1-alpha)*love.timer._adt
   love.update(dt)
end
function love.update( dt ) end -- dummy
function love.timer.getDelta() return love.timer._dt end
function love.timer.getAverageDelta() return love.timer._adt end
function love.timer.getFPS() return 1/love.timer._adt end
function love.timer.getTime() return love.timer._edt end


--[[
-- Window
--]]
love.window = {}
function love.window.setMode( width, height, flags ) return end


--[[
-- Events
--]]
love.event = {}
function love.event.quit( exitstatus ) tk.customDone() end


--[[
-- Filesystem
--]]
love.filesystem = {}
function love.filesystem.getInfo( path, filtertype )
   local ftype = file.filetype( path )
   if ftype == "directory" then
      return { type = ftype }
   elseif ftype == "file" then
      local info = { type = ftype }
      local f = file.new( path )
      f:open('r')
      info.size = f:getSize()
      f:close()
      return info
   end
   return nil
end
function love.filesystem.newFile( filename )
   return file.new( love.basepath..filename )
end
function love.filesystem.read( name, size )
   local f = file.new( name )
   f:open('r')
   local buf,len
   if size then
      buf,len = f:read( size )
   else
      buf,len = f.read()
   end
   f:close()
   return buf, len
end


--[[
-- Mouse
--]]
-- Internal function that connects to Naev
love.mouse = {}
love.mouse.x = 0
love.mouse.y = 0
love.mouse.lx = 0
love.mouse.ly = 0
love.mouse.down = {}
local function _mouse( x, y, mtype, button )
   y = love.h-y-1
   love.mouse.x = x
   love.mouse.y = y
   if mtype==1 then
      love.mouse.down[button] = true
      love.mousepressed( x, y, button, false )
   elseif mtype==2 then
      love.mouse.down[button] = false
      love.mousereleased( x, y, button, false )
   elseif mtype==3 then
      local dx = x - love.mouse.lx
      local dy = y - love.mouse.ly
      love.mouse.lx = x
      love.mouse.ly = y
      love.mousemoved( x, y, dx, dy, false )
   end
   return true
end
function love.mouse.getX() return love.mouse.x end
function love.mouse.getY() return love.mouse.y end
function love.mouse.isDown( button ) return love.mouse.down[button]==true end
function love.mouse.setVisible( visible ) end -- unused
function love.mousemoved( x, y, dx, dy, istouch ) end -- dummy
function love.mousepressed( x, y, button, istouch ) end -- dummy
function love.mousereleased( x, y, button, istouch ) end -- dummy


--[[
-- Keyboard
--]]
love.keyboard = {}
love.keyboard._keystate = {}
love.keyboard._repeat = false
-- Internal function that connects to Naev
local function _keyboard( pressed, key, mod )
   local k = string.lower( key )
   love.keyboard._keystate[ k ] = pressed
   if pressed then
      love.keypressed( k, k, false )
   else
      love.keyreleased( k, k )
   end
   if key == "Q" then
      tk.customDone()
   end
   return true
end
function love.keypressed( key, scancode, isrepeat ) end -- dummy
function love.keyreleased( key, scancode ) end -- dummy
function love.keyboard.isDown( key )
   return (love.keyboard._keystate[ key ] == true)
end
function love.keyboard.setKeyRepeat( enable )
   love.keyboard._repeat = enable
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
   gfx.renderRect( x, y, w, h, love.graphics._bgcol )
   love.draw()
end
love.graphics = {}
love.graphics._dx = 0
love.graphics._dy = 0
love.graphics._font = font.new( 12 )
love.graphics._bgcol = colour.new( 0, 0, 0, 1 )
love.graphics._fgcol = colour.new( 1, 1, 1, 1 )
-- Helper functions
local function _mode(m)
   if     m=="fill" then return false
   elseif m=="line" then return true
   else   error( string.format(_("Unknown fill mode '%s'"), mode ) )
   end
end
local function _xy( x, y, w, h )
   return love.x+love.graphics._dx+x, love.y+(love.h-y-h-love.graphics._dy)
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
-- Drawable class
love.graphics.Drawable = inheritsFrom( nil )
function love.graphics.Drawable:getDimensions() error('unimplemented') end
-- Image class
love.graphics.Image = inheritsFrom( love.graphics.Drawable )
function love.graphics.newImage( filename )
   local t = love.graphics.Image.new()
   t.tex = tex.open( love.filesystem.newFile( filename ) )
   return t
end
function love.graphics.Image:setFilter( min, mag ) end
function love.graphics.Image:getDimensions()
   local w,h = self.tex:dim()
   return w,h
end
function love.graphics.Image:getWidth()
   local w,h = self.tex:dim()
   return w
end
function love.graphics.Image:getHeight()
   local w,h = self.tex:dim()
   return h
end
function love.graphics.Image:_draw( ... )
   local arg = {...}
   local w,h = self.tex:dim()
   local x,y,r,sx,sy,tx,ty,tw,th
   if type(arg[1])=='number' then
      -- x, y, r, sx, sy
      x,y = _xy(arg[1],arg[2],w,h)
      r = arg[3] or 0
      sx = arg[4] or 1
      sy = arg[5] or sx
      tx = 0
      ty = 0
      tw = 1
      th = 1
   else
      -- quad, x, y, r, sx, sy
      local q = arg[1]
      x = arg[2]
      y = arg[3]
      r = arg[4] or 0
      x = arg[5] or 1
      y = arg[6] or sx
      tx = q.x
      ty = q.y
      tw = q.w
      th = q.h
      x,y = _xy(x,y,w,h)
   end
   w = w*sx
   h = h*sy
   y = y - (h*(1-sy)) -- correct scaling
   gfx.renderTexRaw( self.tex, x, y, w, h, 1, 1, tx, ty, tw, th, love.graphics._fgcol, r )
end
-- Quad class
love.graphics.Quad = inheritsFrom( love.graphics.Drawable )
function love.graphics.newQuad( x, y, width, height, sw, sh )
   local q = love.graphics.Drawable.new()
   q.x = x/sw
   q.y = y/sw
   q.w = width/sw
   q.h = height/sh
   q.quad = true
   return q
end
-- SpriteBatch class
love.graphics.SpriteBatch = inheritsFrom( love.graphics.Drawable )
function love.graphics.newSpriteBatch( image, maxsprites, usage  )
   local batch = love.graphics.Drawable.new()
   return batch
end
-- Other classes
function love.graphics.getWidth()  return love.w end
function love.graphics.getHeight() return love.h end
function love.graphics.origin()
   love.graphics._dx = 0
   love.graphics._dy = 0
end
function love.graphics.translate( dx, dy )
   love.graphics._dx = love.graphics._dx + dx
   love.graphics._dy = love.graphics._dy + dy
end
function love.graphics.getBackgroundColor() return _gcol( self.graphics._bgcol ) end
function love.graphics.setBackgroundColor( red, green, blue, alpha )
   love.graphics._bgcol = _scol( red, green, blue, alpha )
end
function love.graphics.getColor() return _gcol( self.graphics._fgcol ) end
function love.graphics.setColor( red, green, blue, alpha )
   love.graphics._fgcol = _scol( red, green, blue, alpha )
end
function love.graphics.rectangle( mode, x, y, width, height )
   x,y = _xy(x,y,width,height)
   gfx.renderRect( x, y, width, height, love.graphics._fgcol, _mode(mode) )
end
function love.graphics.circle( mode, x, y, radius )
   x,y = _xy(x,y,0,0)
   gfx.renderCircle( x, y, radius, love.graphics._fgcol, _mode(mode) )
end
--function love.graphics.draw( drawable, x, y, r, sx, sy )
function love.graphics.draw( drawable, ... )
   drawable:_draw( ... )
end
function love.graphics.print( text, x, y  )
   x,y = _xy(x,y,limit,love.graphics._font:height())
   gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol )
end
function love.graphics.printf( text, x, y, limit, align )
   x,y = _xy(x,y,limit,love.graphics._font:height())
   if align=="left" then
      gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol, limit, false )
   elseif align=="center" then
      gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol, limit, true )
   elseif align=="right" then
      local w = gfx.printDim( false, text, limit )
      local off = limit-w
      gfx.printf( love.graphics._font, text, x+off, y, love.graphics._fgcol, w, false )
   end
end
function love.graphics.newFont( file, size )
   if size==nil then
      return font.new( file )
   elseif type(file)=="userdata" then
      return file
   else
      return font.new( file, size )
   end
end
function love.graphics.setFont( fnt )
   love.graphics._font = fnt
end
function love.graphics.setNewFont( file, size )
   love.graphics._font = love.graphics.newFont( file, size )
   return love.graphics._font
end
-- unimplemented
function love.graphics.setDefaultFilter( min, mag, anisotropy ) end
function love.graphics.setLineStyle( style ) end


--[[
-- Math
--]]
love.math = {}
love.math.RandomGenerator = inheritsFrom( nil )
function love.math.newRandomGenerator( low, high )
   -- TODO implement a real one?
   return love.math.RandomGenerator.new()
end
function love.math.RandomGenerator:random( min, max )
   return love.math.random( min, max )
end
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
-- Audio
--]]
love.audio = {}
function love.audio.newSource( filename, type )
   return audio.new( filename, type )
end
function love.audio.setVolume( volume ) end -- Don't allow setting master volume
function love.audio.getVolume( volume )
   return audio.getVolume()
end


--[[
-- Sound
--]]
love.sound = {}
function love.sound.newSoundData( filename ) end


--[[
-- Initialize
--]]
package.path = package.path..string.format(";?.lua", path)
function love.exec( path )
   local info = love.filesystem.getInfo( path )
   if info then
      if info.type == "directory" then
         love.basepath = path.."/" -- Allows loading files relatively
         package.path = package.path..string.format(";%s/?.lua", path)
         -- Run conf if exists
         if love.filesystem.getInfo( path.."/conf.lua" ) ~= nil then
            require( path.."/conf" )
         end
         require( path.."/main" )
      elseif info.type == "file" then
         require( path )
      else
         error( string.format( _("'%s' is an unknown filetype '%s'", path, info.type) ) )
      end
   else
      local npath = path..".lua" 
      info = love.filesystem.getInfo( npath )
      if info and info.type == "file" then
         require( path )
      else
         error( string.format( _("'%s' is not a valid love2d game!"), path) )
      end
   end

   -- Only stuff we care about atm
   local t = {}
   t.audio = {}
   t.window = {}
   t.window.title = "LÖVE" -- The window title (string)
   t.window.width = 800    -- The window width (number)
   t.window.height = 600   -- The window height (number)
   t.modules = {}

   -- Configure
   love.conf(t)

   -- Set properties
   love.title = t.window.title
   love.w = t.window.width
   love.h = t.window.height

   -- Run set up function defined in Love2d spec
   love.load()

   -- Actually run in Naev
   tk.custom( love.title, love.w, love.h, _update, _draw, _keyboard, _mouse )
end

-- Fancy API so you can do `love = require 'love'`
return love
