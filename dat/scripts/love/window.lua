--[[
-- Window
--]]
local love = require 'love'

local window = {}

window._lastw = love._default.w
window._lasth = love._default.h

function window.setIcon( imagedata )
   love.icon = imagedata
   return true
end
function window.getIcon() return love.icon end
function window.setTitle( title )
   love.title = title
   if love._started then
      naev.tk.customRename( love.title )
   end
end
function window.getTitle() return love.title end
function window.setMode( width, height, flags )
   local fullscreen
   if type(flags)=="table" then
      fullscreen = flags.fullscreen or false
   end
   if fullscreen and love.fullscreen then return true end

   love.fullscreen = fullscreen
   if love._started then
      love.tk.customFullscreen( love.fullscreen )
      if fullscreen then
         window._lastw = love.w
         window._lasth = love.h
         love.w, love.h = naev.tk.customSize()
      else
         love.w = width
         love.h = height
         love.tk.customResize( love.w, love.h )
      end
   else
      if fullscreen then
         window._lastw = love.w
         window._lasth = love.h
         love.w, love.h = naev.gfx.dim()
      else
         love.w = width
         love.h = height
         if love.w <= 0 then love.w = love._default.w end
         if love.h <= 0 then love.h = love._default.h end
      end
   end
   return true
end
function window.getDisplayCount() return 1 end
function window.getDisplayName( displayindex ) return "Naev" end
function window.getDesktopDimensions() return naev.gfx.dim() end
--function window.getDimensions() return love.w, love.h end -- Removed in Love2D 10.0
function window.getWidth() return love.w end
function window.getHeight() return love.h end
function window.getDPIScale() return 1 end -- TODO return scaling factor?
function window.getMode()
   return love.w, love.h, { fullscreen=love.fullscreen, vsync=1, resizeable=false, borderless = false, centered=true, display=1, msaa=0 }
end
function window.setFullscreen( fullscreen )
   -- Skip unnecessary changing
   if (fullscreen and love.fullscreen) or (not fullscreen and not love.fullscreen) then return true end
   love.fullscreen = fullscreen
   if love._started then
      naev.tk.customFullscreen( love.fullscreen )
      if not love.fullscreen then
         naev.tk.customResize( window._lastw, window._lasth )
      end
      love.w, love.h = naev.tk.customSize()
   else
      if love.fullscreen then
         love.w, love.h = naev.gfx.dim()
      else
         love.w = window._lastw
         love.h = window._lasth
      end
   end
   return true
end
function window.getFullscreen( fullscreen ) return love.fullscreen end
function window.hasFocus() return love._focus end
function window.hasMouseFocus() return love._focus end
function window.showMessageBox( title, message, ... )
   local arg = {...}
   love._focus = false
   if type(arg[1])=="string" then
      tk.msg( title, message )
   else
      local choice = tk.choice( title, message, unpack(arg) )
      love._focus = true
      return choice
   end
   love._focus = true
   return true
end

return window
