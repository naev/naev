--[[

Love2d API in Naev!!!!!
Meant to be loaded as a library to run Love2d stuff out of the box.

Example usage would be as follows:
"""
love = require 'love'
love.exec( 'pong' ) -- Will look for pong.lua or pong/main.lua
"""

--]]
local love = {
   _basepath = "",
   _version_major = 11,
   _version_minor = 1,
   _version_patch = 3,
   _codename = "naev",
   _default = {
      title = "LÖVE",
      w = 800,
      h = 600,
      fullscreen = false,
   },
}
function love._unimplemented() error(_("unimplemented")) end

--[[
-- Dummy game-defined functions
--]]
function love.conf(t) end -- dummy
function love.load() end --dummy
function love.draw() end -- dummy
function love.update( dt ) end -- dummy
function love.keypressed( key, scancode, isrepeat ) end -- dummy
function love.keyreleased( key, scancode ) end -- dummy
function love.mousemoved( x, y, dx, dy, istouch ) end -- dummy
function love.mousepressed( x, y, button, istouch ) end -- dummy
function love.mousereleased( x, y, button, istouch ) end -- dummy


--[[
-- Base
--]]
function love.getVersion()
   return love._version_major, love._version_minor, love._version_patch, love._codename
end


--[[
-- Internal function that connects to Naev
--]]
local function _draw( x, y, w, h )
   love.x = x
   love.y = y
   love.w = w
   love.h = h
   if love.graphics then
      love.graphics.origin()
      love.graphics.clear()
   end
   love.draw()
end
local function _update( dt )
   if love.keyboard and love.keyboard._repeat then
      for k,v in pairs(love.keyboard._keystate) do
         if v then
            love.keypressed( k, k, true )
         end
      end
   end
   if not love.timer then
      love.update(0)
      return
   end
   love.timer._edt = love.timer._edt + dt
   love.timer._dt = dt
   local alpha = 0.1
   love.timer._adt = alpha*dt + (1-alpha)*love.timer._adt
   love.update(dt)
end
local function _mouse( x, y, mtype, button )
   if not love.mouse then return end
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
local function _keyboard( pressed, key, mod )
   if not love.keyboard then return end
   local k = string.lower( key )
   love.keyboard._keystate[ k ] = pressed
   if pressed then
      love.keypressed( k, k, false )
   else
      love.keyreleased( k, k )
   end
   if k == "escape" then
      naev.tk.customDone()
   end
   return true
end


--[[
-- Initialize
--]]
function love.exec( path )
   -- Save path to restore it later
   love._path = package.path

   package.path = package.path..";?.lua"

   love._focus = false
   love._started = false
   love.filesystem = require 'love.filesystem'

   local info = love.filesystem.getInfo( path )
   local confpath, mainpath
   if info then
      if info.type == "directory" then
         love._basepath = path.."/" -- Allows loading files relatively
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
   t.window.fullscreen = love._default.fullscreen
   t.modules = {
         audio = true,
         data = true,
         event = true,
         font = true,
         graphics = true,
         image = true,
         joystick = true,
         keyboard = true,
         math = true,
         mouse = true,
         physics = true,
         sound = true,
         system = true,
         thread = true,
         timer = true,
         touch = true,
         video = true,
         window = true
      }

   -- Configure
   if confpath ~= nil then
      require( confpath )
   end
   love.conf(t)

   -- Load stuff
   for m,v in pairs(t.modules) do
      if v then
         love[m] = require('love.'..m)
      end
   end

   -- Set properties
   love.title = t.window.title
   love.w = t.window.width
   love.h = t.window.height
   love.fullscreen = t.window.fullscreen
   if love.fullscreen then
      love.w, love.h = love.window.getDesktopDimensions()
   end

   -- Run set up function defined in Love2d spec
   require( mainpath )
   love.load()

   -- Actually run in Naev
   if love.fullscreen then
      love.w = -1
      love.h = -1
   end
   love._focus = true
   love._started = true
   naev.tk.custom( love.title, love.w, love.h, _update, _draw, _keyboard, _mouse )
   -- Doesn't actually get here until the dialogue is closed

   -- Reset libraries that were potentially crushed
   for k,v in pairs(naev) do _G[k] = v end
   -- Restore the package.path
   package.path = love._path
end

-- Fancy API so you can do `love = require 'love'`
return love
