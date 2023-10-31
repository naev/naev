--[[

Love2d API in Naev!!!!!
Meant to be loaded as a library to run Love2d stuff out of the box.

Example usage would be as follows:
"""
love = require 'love'
love.exec( 'pong' ) -- Will look for pong.lua or pong/main.lua
"""

--]]

-- luacheck: globals love (We are *implementing* the love2d API.)
love = {
   _basepath = "",
   _version_major = 11,
   _version_minor = 4,
   _version_patch = 0,
   _codename = "naev",
   _default = {
      title = "LÖVE",
      w = 800,
      h = 600,
      fullscreen = false,
   },
   x = 0,
   y = 0,
   s = 1,
}
love.w = love._default.w
love.h = love._default.h
function love._unimplemented() error(_("unimplemented")) end
local function _clearfuncs()
   local function _noop() end
   local f = {
      "conf",
      "load",
      "draw",
      "update",
      "keypressed",
      "keyreleased",
      "mousemoved",
      "mousepressed",
      "mousereleased",
      "wheelmoved",
      "resize",
      "textinput",
   }
   for k,v in ipairs(f) do
      love[v] = _noop
   end
end
_clearfuncs()
local function _setdefaults()
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
   return t
end


--[[
-- Base
--]]
function love.getVersion()
   return love._version_major, love._version_minor, love._version_patch, love._codename
end
function love.origin()
   local nw, nh = naev.gfx.dim()
   love.x = 0
   love.y = 0
   love.w = nw
   love.h = nh
   local lg = require "love.graphics"
   lg.origin()
end
-- Non-standard function to refresh the screen
function love.refresh ()
   tk.refresh()
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
   if not love.mouse then return false end
   if mtype ~= 4 then -- if not a mouse-wheel event
      y = love.h-y-1
      love.mouse.x = x
      love.mouse.y = y
   end
   if mtype==1 then
      love.mouse.down[button] = true
      return love.mousepressed( x, y, button, false )
   elseif mtype==2 then
      love.mouse.down[button] = false
      return love.mousereleased( x, y, button, false )
   elseif mtype==3 then
      local dx = x - love.mouse.lx
      local dy = y - love.mouse.ly
      love.mouse.lx = x
      love.mouse.ly = y
      return love.mousemoved( x, y, dx, dy, false )
   elseif mtype==4 then
      return love.wheelmoved( x, y )
   end
end
local function _keyboard( pressed, key, _mod, isrepeat )
   if not love.keyboard then return false end
   local k = string.lower( key )
   love.keyboard._keystate[ k ] = pressed
   if pressed then
      return love.keypressed( k, k, isrepeat )
   else
      return love.keyreleased( k, k )
   end
end
local function _resize( w, h )
   if w ~= 0 and h ~= 0 and (w ~= love.w or h ~= love.h) then
      naev.tk.customFullscreen( love.fullscreen )
      return love.resize( w, h )
   end
end
function __resize( _w, _h )
   love.origin()
end
local function _textinput( str )
   return love.textinput( str )
end

--[[
-- Initialize
--]]
function love.exec( path )
   if love._started then
      error(_("can only run one Love2D instance at a time!"))
   end

   -- Save path to restore it later
   love._path = package.path

   -- only add to path if not there, saves path pollution if crashing
   local function addtopath( newpath )
      local id = string.find( package.path, newpath, 1, true )
      if id == nil then
         package.path = package.path..newpath
      end
   end
   addtopath(";?.lua")

   love._focus = false
   love._started = false
   love.filesystem = require 'love.filesystem'

   local info = love.filesystem.getInfo( path )
   local confpath, mainpath
   if info then
      if info.type == "directory" then
         love._basepath = path.."/" -- Allows loading files relatively
         addtopath(string.format(";%s/?.lua", path))
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

   -- Set defaults
   _clearfuncs()
   local t = _setdefaults()

   local function dolua( scriptpath )
      _LOADED[scriptpath] = nil -- reset loadedness
      require(scriptpath)
   end

   -- Configure
   if confpath ~= nil then
      dolua( confpath )
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
   dolua( mainpath )
   love.load()

   -- Actually run in Naev
   if love.fullscreen then
      love.w = -1
      love.h = -1
   end
   love._focus = true
   love._started = true
   naev.tk.custom( love.title, love.w, love.h, _update, _draw, _keyboard, _mouse, _resize, _textinput, t.drawondemand )
   -- Doesn't actually get here until the dialogue is closed
   love._started = false

   -- Reset libraries that were potentially crushed
   for k,v in pairs(naev) do _G[k] = v end
   -- Restore the package.path
   package.path = love._path
end
function love.run()
   if love._started then
      error(_("can only run one Love2D instance at a time!"))
   end

   love._focus = false
   love._started = false

   -- Set up defaults
   local t = _setdefaults()

   -- Configure
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
   love.load()

   -- Actually run in Naev
   if love.fullscreen then
      love.w = -1
      love.h = -1
   end
   love._focus = true
   love._started = true
   naev.tk.custom( love.title, love.w, love.h, _update, _draw, _keyboard, _mouse, _resize, _textinput )
   -- Doesn't actually get here until the dialogue is closed
   love._started = false

   -- Reset libraries that were potentially crushed
   for k,v in pairs(naev) do _G[k] = v end
end

-- Fancy API so you can do `love = require 'love'`
return love
