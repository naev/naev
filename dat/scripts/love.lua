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
love._codename = "naev"
love._default = {
   title = "LÖVE",
   w = 800,
   h = 600
}

-- Dummy user-defined functions
function love.conf(t) end -- dummy
function love.load() end --dummy


--[[
-- Base
--]]
function love.getVersion()
   return love._version_major, love._version_minor, love._version_patch, love._codename
end
love.Object = inheritsFrom( nil )
love.Object._type = "Object"
function love.Object:type() return self._type end
function love.Object:typeOf( name ) return self._type==name end



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
function love.window.setTitle( title )
   if love._started then
      naev.tk.customRename( title )
   else
      love.title = title
   end
end
function love.window.setMode( width, height, flags )
   if love._started then
      -- TODO
   else
      love.w = width
      love.h = height
      if love.w <= 0 then love.w = love._default.w end
      if love.h <= 0 then love.h = love._default.h end
   end
end
function love.window.getDesktopDimensions()
   return love.w, love.h
end
function love.window.getDPIScale()
   return 1
end
function love.window.getMode()
   return love.w, love.h, { fullscreen=false, vsync=1, resizeable=false, borderless = false, centered=true, display=1, msaa=0 }
end


--[[
-- Events
--]]
love.event = {}
function love.event.quit( exitstatus ) naev.tk.customDone() end


--[[
-- Filesystem
--]]
love.filesystem = {}
function love.filesystem.getInfo( path, filtertype )
   local ftype = naev.file.filetype( path )
   if ftype == "directory" then
      return { type = ftype }
   elseif ftype == "file" then
      local info = { type = ftype }
      local f = naev.file.new( path )
      f:open('r')
      info.size = f:getSize()
      f:close()
      return info
   end
   return nil
end
function love.filesystem.newFile( filename )
   return naev.file.new( love.basepath..filename )
end
function love.filesystem.read( name, size )
   local f = naev.file.new( name )
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
      naev.tk.customDone()
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
-- Image
--]]
love.image = {}
love.image.ImageData = inheritsFrom( love.Object )
love.image.ImageData._type = "ImageData"
function love.image.newImageData( ... )
   local arg = {...}
   local w, h, d
   local t = type(arg[1])
   if t=="number" then
      w = arg[1]
      h = arg[2]
      d = data.new( w*h*4, "number" )
   elseif t=="string" then
      local f = love.filesystem.newFile(arg[1])
      d, w, h = naev.tex.readData( f )
   else
      error( 'unimplemented' )
   end
   local newd = love.image.ImageData.new()
   newd.w = w
   newd.h = h
   newd.d = d
   return newd
end
function love.image.ImageData:getSize()
   return self.d:getSize()
end
function love.image.ImageData:getString()
   return self.d:getString()
end
local function _id_pos(self,x,y) return 4*(y*self.w+x) end
function love.image.ImageData:getDimensions() return self.w, self.h end
function love.image.ImageData:getWidth() return self.w end
function love.image.ImageData:getHeight() return self.h end
function love.image.ImageData:getPixel( x, y )
   local pos = _id_pos(self,x,y)
   local r = self.d:get( pos+0 )
   local g = self.d:get( pos+1 )
   local b = self.d:get( pos+2 )
   local a = self.d:get( pos+3 )
   return r, g, b, a
end
function love.image.ImageData:setPixel( x, y, r, g, b, a )
   local pos = _id_pos(self,x,y)
   self.d:set( pos+0, r )
   self.d:set( pos+1, g )
   self.d:set( pos+2, b )
   self.d:set( pos+3, a )
end
function love.image.ImageData:paste( source, dx, dy, sx, sy, sw, sh )
   -- probably very slow
   for x = 0,sw-1 do
      for y = 0,sh-1 do
         self:setPixel( dx+x, dy+y, source:getPixel( sx+x, sy+y ) )
      end
   end
end


--[[
-- Math
--]]
love.math = {}
love.math.RandomGenerator = inheritsFrom( love.Object )
love.math.RandomGenerator._type = "RandomGenerator"
function love.math.newRandomGenerator( low, high )
   -- TODO implement a real one?
   return love.math.RandomGenerator.new()
end
function love.math.RandomGenerator:random( min, max )
   return love.math.random( min, max )
end
function love.math.random( min, max )
   if min == nil then
      return naev.rnd.rnd()
   elseif max == nil then
      return naev.rnd.rnd( min-1 )+1
   else
      return naev.rnd.rnd( min, max )
   end
end


--[[
-- Audio
--]]
love.audio = {}
love.audio.Source = inheritsFrom( love.Object )
love.audio.Source._type = "Source"
function love.audio.newSource( filename, type )
   local s = love.audio.Source.new()
   --s.a = audio.new( filename, type )
   return s
end
function love.audio.Source:play() return true end
function love.audio.Source:pause() end
function love.audio.Source:stop() end
function love.audio.Source:isPlaying() return false end 
function love.audio.Source:setVolume( volume ) end
function love.audio.Source:getVolume() return audio.getVolume() end
function love.audio.Source:setLooping( looping ) end
function love.audio.Source:setPitch( pitch ) end
function love.audio.Source:setPosition( x, y, z ) end
function love.audio.Source:setAttenuationDistances( ref, max ) end
function love.audio.setVolume( volume ) end -- Don't allow setting master volume
function love.audio.getVolume( volume )
   return audio.getVolume()
end
function love.audio.setPosition( x, y, z ) end


--[[
-- Sound
--]]
love.sound = {}
function love.sound.newSoundData( filename ) end


--[[
-- Graphics
--]]
-- Internal function that connects to Naev
local function _draw( x, y, w, h )
   love.x = x
   love.y = y
   love.w = w
   love.h = h
   naev.gfx.renderRect( x, y, w, h, love.graphics._bgcol )
   love.draw()
end
love.graphics = require 'love/graphics'
function love.draw() end -- dummy


--[[
-- Initialize
--]]
package.path = package.path..string.format(";?.lua", path)
function love.exec( path )
   love._started = false

   local info = love.filesystem.getInfo( path )
   local confpath, mainpath
   if info then
      if info.type == "directory" then
         love.basepath = path.."/" -- Allows loading files relatively
         package.path = package.path..string.format(";%s/?.lua", path)
         -- Run conf if exists
         if love.filesystem.getInfo( path.."/conf.lua" ) ~= nil then
            confpath = path.."/conf"
         end
         mainpath = path.."/main"
      elseif info.type == "file" then
         mainpath = path
      else
         error( string.format( _("'%s' is an unknown filetype '%s'", path, info.type) ) )
      end
   else
      local npath = path..".lua"
      info = love.filesystem.getInfo( npath )
      if info and info.type == "file" then
         mainpath = path
      else
         error( string.format( _("'%s' is not a valid love2d game!"), path) )
      end
   end

   -- Only stuff we care about atm
   local t = {}
   t.audio = {}
   t.window = {}
   t.window.title = love._default.title -- The window title (string)
   t.window.width = love._default.w -- The window width (number)
   t.window.height = love._default.h -- The window height (number)
   t.modules = {}

   -- Configure
   if confpath ~= nil then
      require( confpath )
   end
   love.conf(t)

   -- Set properties
   love.title = t.window.title
   love.w = t.window.width
   love.h = t.window.height

   -- Run set up function defined in Love2d spec
   require( mainpath )
   love.load()

   -- Actually run in Naev
   naev.tk.custom( love.title, love.w, love.h, _update, _draw, _keyboard, _mouse )
   love._started = true

   -- Reset libraries that were potentially crushed
   for k,v in pairs(naev) do _G[k] = v end
end

-- Fancy API so you can do `love = require 'love'`
return love
