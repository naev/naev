--[[
   Implementation of parts of the Naev toolkit... in Lua!

   Based on Love2D API
   @module luatk
--]]
local lg = require 'love.graphics'
local le = require 'love.event'
local lk = require 'love.keyboard'
local utf8 = require 'utf8'
local vn = require "vn"

local luatk = {
   _windows = {},

   colour = {
      bg       = { 0.2,  0.2,  0.2  },
      outline  = { 0.5,  0.5,  0.5  },
      dark     = { 0.05, 0.05, 0.05 },
      text     = { 0.95, 0.95, 0.95 },
      selected = { 0.3,  0.3,  0.3  },
   },
   button = {
      colour = {
         text              = { 0.7,  0.7,  0.7  },
         outline           = { 0.15, 0.15, 0.15 },
         mouseover         = { 0.3,  0.3,  0.3  },
         mousedown         = { 0.35, 0.35, 0.35 },
         bg                = { 0.25, 0.25, 0.25 },
         disabled_outline  = { 0.2,  0.2,  0.2  },
         disabled_bg       = { 0.2,  0.2,  0.2  },
      },
   },
   scrollbar = {
      colour = {
         value    = { 0.95, 0.95, 0.95 },
         label    = { 0.6,  0.6,  0.6  },
         bg       = { 0.0,  0.0,  0.0  },
         fg       = { 0.5,  0.5,  0.5  },
         outline  = { 0.05, 0.05, 0.05 },
      },
   },
   input = {
      colour = {
         text     = { 0.2,  0.8,  0.1  },
      },
   },
   _deffont = nil,
}

--[[
-- Global functions
--]]
--[[--
Gets whether or not the luatk has a window open.
--]]
function luatk.isOpen ()
   return #luatk._windows>0
end
--[[--
Runs the luatk. Should be used after the windows are set up.
--]]
function luatk.run ()
   local f = lg.getFont()
   local o = f:getOutline()
   f:setOutline(1)
   luatk._love = true
   love.exec( 'scripts/luatk' )  -- luacheck: ignore
   luatk._love = false
   f:setOutline(o)
end
--[[--
Creates a custom state inside a vn state.

A full example is shown below.
@code
vn.clear()
vn.scene()
vn.transition()
vn.na("This is a test")
luatk.vn( function ()
   local wdw = luatk.msgInput( "Test", "Just a test", 50, function( str )
      print( str )
   end )
end )
vn.na("That was all!")
vn.run()
@endcode

   @tparam function setup Function to set up the luatk.
--]]
function luatk.vn( setup )
   local s = vn.custom()
   s._init = function ()
      setup()
   end
   s._draw = function ()
      return luatk.draw()
   end
   s._mousepressed = function( _self, mx, my, button )
      return luatk.mousepressed( mx, my, button )
   end
   s._mousereleased = function( _self, mx, my, button )
      return luatk.mousereleased( mx, my, button )
   end
   s._mousemoved = function( _self, mx, my, dx, dy )
      return luatk.mousemoved( mx, my, dx, dy )
   end
   s._keypressed = function( _self, key )
      return luatk.keypressed( key )
   end
   s._textinput = function( _self, str )
      return luatk.textinput( str )
   end
   local function _update( self, dt )
      if #luatk._windows<=0 then
         self.done = true
         return
      end
      luatk.update( dt )
   end
   -- See reason for this hack in dat/scripts/luatk/main.lua
   s._update = function( self, dt )
      -- Start focus
      local wdw = luatk._windows[ #luatk._windows ]
      if not wdw then
         return
      end
      for _k,wgt in ipairs(wdw._widgets) do
         if wgt.focus then
            wgt:focus()
         end
      end

      s._update = _update
      _update( self, dt )
   end
   return s
end
--[[--
Closes the toolkit and all windows.
--]]
function luatk.close ()
   luatk._windows = {}
   lk.setTextInput( false ) -- Hack as we can't use __gc
   if luatk._love then
      le.quit()
   end
end
--[[--
Sets the default font to use for the toolkit.

   @tparam font Font to set as default.
--]]
function luatk.setDefaultFont( font )
   luatk._deffont = font
end

--[[
-- Important functions
--]]
--[[--
Draws the luatk toolkit.

Only to be used when running the toolkit outside of luatk.run.
--]]
function luatk.draw()
   for _k,wdw in ipairs(luatk._windows) do
      wdw:draw()
   end
end
--[[--
Updates the luatk toolkit.

Only to be used when running the toolkit outside of luatk.run.
   @tparam number dt Number of seconds since last update.
--]]
function luatk.update(dt)
   for _k,wdw in ipairs(luatk._windows) do
      wdw:update(dt)
   end
end
local function _checkbounds( b, mx, my )
   return not (mx < b.x or mx > b.x+b.w or my < b.y or my > b.y+b.h)
end
--[[--
Handles mouse clicks.

Only to be used when running the toolkit outside of luatk.run.
   @tparam number mx X coordinates of the mouse click.
   @tparam number my Y coordinates of the mouse click.
   @tparam integer button Number of the button pressed.
   @treturn boolean true if the event was used, false otherwise.
--]]
function luatk.mousepressed( mx, my, button )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw or not _checkbounds(wdw,mx,my) then return false end
   local x, y = mx-wdw.x, my-wdw.y

   for _k,wgt in ipairs(wdw._widgets) do
      if _checkbounds(wgt,x,y) then
         wgt._pressed = true
         if wgt.pressed and wgt:pressed( x-wgt.x, y-wgt.y, button ) then
            return true
         end
      end
   end

   return false
end
--[[--
Handles mouse releases

Only to be used when running the toolkit outside of luatk.run.
   @tparam number mx X coordinates of the mouse released position.
   @tparam number my Y coordinates of the mouse released position.
   @tparam integer button Number of the button released.
   @treturn boolean true if the event was used, false otherwise.
--]]
function luatk.mousereleased( mx, my, button )
   local wdw = luatk._windows[ #luatk._windows ]
   local x, y = mx-wdw.x, my-wdw.y

   for _k,wgt in ipairs(wdw._widgets) do
      local inbounds = _checkbounds(wgt,x,y)
      if wgt._pressed and inbounds and wgt.clicked then
         wgt:clicked( x-wgt.x, y-wgt.y, button )
      end
      wgt._pressed = false
      if wgt.released then
         wgt:released()
      end
      if inbounds then
         wgt.mouseover = true
      end
   end

   return false
end
--[[--
Handles mouse motion.

Only to be used when running the toolkit outside of luatk.run.
   @tparam number mx X coordinates of the mouse released position.
   @tparam number my Y coordinates of the mouse released position.
   @treturn boolean true if the event was used, false otherwise.
--]]
function luatk.mousemoved( mx, my )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw or not _checkbounds(wdw,mx,my) then return false end
   local x, y = mx-wdw.x, my-wdw.y

   for _k,wgt in ipairs(wdw._widgets) do
      local inbounds = _checkbounds( wgt, x, y )
      if not wgt._pressed then
         wgt.mouseover = inbounds
      end
      if (inbounds or wgt._pressed) and wgt.mmoved then
         wgt.mmoved( wgt, x-wgt.x, y-wgt.y )
      end
   end

   return false
end
--[[--
Handles key presses.

   @treturn string key Name of the key pressed.
--]]
function luatk.keypressed( key )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw then return false end

   -- Custom keypress events
   if wdw.keypressed and wdw.keypressed( key ) then
      return true
   end

   if key=="return" then
      if wdw.accept and wdw:accept() then
         return true
      end
   elseif key=="escape" then
      if wdw.cancel and wdw:cancel() then
         return true
      end
   end

   for _k,wgt in ipairs(wdw._widgets) do
      -- TODO proper focus model
      if wgt.keypressed then
         if wgt:keypressed( key ) then
            return true
         end
      end
   end

   return false
end
--[[--
Handles text input.

   @treturn string key Name of the key pressed.
--]]
function luatk.textinput( str )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw then return false end

   for _k,wgt in ipairs(wdw._widgets) do
      -- TODO proper focus model
      if wgt.textinput then
         if wgt:textinput( str ) then
            return true
         end
      end
   end

   return false
end

--[[
-- Helper functions
--]]
local scrollbar_h = 30 -- bar height
local function drawScrollbar( x, y, w, h, pos )
   lg.setColor( luatk.scrollbar.colour.bg )
   lg.rectangle( "fill", x, y, w, h )

   -- Scrollbar
   local sy = y + (h-scrollbar_h) * pos
   lg.setColor( luatk.scrollbar.colour.outline )
   lg.rectangle( "fill", x, sy, w, scrollbar_h )
   lg.setColor( luatk.scrollbar.colour.fg )
   lg.rectangle( "fill", x+1, sy, w-2, scrollbar_h-2 )
end

--[[
-- Window class
--]]
luatk.Window = {}
local Window_mt = { __index=luatk.Window }
--[[--
Creates a new window.

   @tparam number|nil x X position of the window or nil to center.
   @tparam number|nil y Y position of the window or nil to center.
   @tparam number w Width to set the window to.
   @tparam number h Height to set the window to.
   @treturn luatk.Window A new luatk window.
--]]
function luatk.newWindow( x, y, w, h )
   local nw, nh = naev.gfx.dim()
   x = x or ((nw-w)/2)
   y = y or ((nh-h)/2)
   local wdw = { x=x, y=y, w=w, h=h, _widgets={} }
   setmetatable( wdw, Window_mt )
   table.insert( luatk._windows, wdw )
   return wdw
end
function luatk.Window:draw()
   local x, y, w, h = self.x, self.y, self.w, self.h

   -- Draw background
   lg.setColor( luatk.colour.bg )
   lg.rectangle( "fill", x, y, w, h )
   lg.setColor( luatk.colour.dark )
   lg.rectangle( "line", x+1, y+1, w-2, h-2 )
   lg.rectangle( "line", x-1, y-1, w+2, h+2 )
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "line", x, y, w, h )

   -- Set scissors
   local scs = lg.getScissor()
   lg.setScissor( x, y, w, h )

   -- Clear depth
   naev.gfx.clearDepth()

   -- Draw widgets ontop
   for _k,wgt in ipairs(self._widgets) do
      wgt:draw( x, y )
   end

   -- Restore scissors
   lg.setScissor( scs )

   -- Draw overlay
   for _k,wgt in ipairs(self._widgets) do
      if wgt.drawover then
         wgt:drawover( x, y )
      end
   end
end
function luatk.Window:update(dt)
   for _k,wgt in ipairs(self._widgets) do
      if wgt.update then
         wgt:update(dt)
      end
   end
   if self.custupdate then
      self.custupdate( dt )
   end
end
--[[--
Sets a custom function to be run each window update.

   @tparam function|nil f Custom function to set. nil disables.
--]]
function luatk.Window:setUpdate( f )
   self.custupdate = f
end
--[[--
Destroys a window.
--]]
function luatk.Window:destroy()
   for k,w in ipairs(luatk._windows) do
      if w==self then
         table.remove(luatk._windows,k)
         return
      end
   end
   if #luatk._widgets <= 0 then
      lk.setTextInput( false ) -- Hack as we can't use __gc
   end
end
--[[--
Sets the accept function to be run when enter is pressed.

   @tparam function func Function to set.
--]]
function luatk.Window:setAccept( func )
   self.accept = func
end
--[[--
Sets the cancel function to be run when escape is pressed.

   @tparam function func Function to set.
--]]
function luatk.Window:setCancel( func )
   self.cancel = func
end
--[[--
Sets a function to handle key input to the window.

   @tparam function func Function to set.
--]]
function luatk.Window:setKeypress( func )
   self.keypressed = func
end
--[[--
Gets the dimensions of the window.

   @treturn number Width of the window.
   @treturn number Height of the window.
--]]
function luatk.Window:getDimensions()
   return self.w, self.h
end

--[[
-- Widget class
--]]
luatk.Widget = {}
luatk.Widget_mt = { __index = luatk.Widget }
function luatk.newWidget( parent, x, y, w, h )
   if x < 0 then
      x = parent.w - w + x
   end
   if y < 0 then
      y = parent.h - h + y
   end
   local wgt = { parent=parent, x=x, y=y, w=w, h=h }
   setmetatable( wgt, luatk.Widget_mt )
   table.insert( parent._widgets, wgt )
   return wgt
end
function luatk.Widget:destroy()
   for k,w in ipairs(self.parent._widgets) do
      if w==self then
         table.remove(self.parent._widgets,k)
         return
      end
   end
end
function luatk.Widget:getDimensions()
   return self.x, self.y, self.w, self.h
end
function luatk.Widget.draw( _self, _x, _y )
end

--[[
-- Button widget
--]]
luatk.Button = {}
setmetatable( luatk.Button, { __index = luatk.Widget } )
luatk.Button_mt = { __index = luatk.Button }
--[[
Creates a new button widget.

   @tparam number x X position of the widget.
   @tparam number y Y position of the widget.
   @tparam number w Width of the widget.
   @tparam number h Height of the widget.
   @tparam string text Text to display on the widget.
   @tparam function handler Function to run when the button widget is clicked.
   @treturn luatk.Button The new button widget.
]]--
function luatk.newButton( parent, x, y, w, h, text, handler )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Button_mt )
   if type(text)=="function" then
      wgt.render  = text
   else
      wgt.text    = text
   end
   wgt.handler = handler

   local font = luatk._deffont or lg.getFont()
   local _sw, wrapped = font:getWrap( text, w )
   wgt.th = font:getHeight() + font:getLineHeight() * (#wrapped-1)

   return wgt
end
function luatk.Button:draw( bx, by )
   local c, fc, outline
   outline = { 0.15, 0.15, 0.15 }
   if self.disabled then
      fc = { 0.5, 0.5, 0.5 }
      c  = { 0.15, 0.15, 0.15 }
   else
      fc = { 0.7, 0.7, 0.7 } -- cFontGrey
      if self._pressed then
         c = { 0.35,  0.35,  0.35  }
      elseif self.mouseover then
         c = { 0.3,  0.3,  0.3  }
      else
         c = { 0.25, 0.25, 0.25 }
      end
   end
   c = self.col or c
   fc = self.fcol or fc
   local x, y, w, h = self:getDimensions()
   x = bx + x
   y = by + y
   local font = luatk._deffont or lg.getFont()
   lg.setColor( outline )
   lg.rectangle( "fill", x-2, y-2, w+4, h+4 )
   lg.setColor( c )
   lg.rectangle( "fill", x, y, w, h )
   lg.setColor( fc )
   if self.text then
      lg.printf( self.text, font, x, y+(h-self.th)/2, w, 'center' )
   else
      self.render( x, y, w, h )
   end
end
function luatk.Button:drawover( bx, by )
   if self.mouseover and self.alt then
      local x, y, w, _h = self:getDimensions()
      luatk.drawAltText( bx+x+w, by+y, self.alt, 300 )
   end
end
function luatk.Button:clicked()
   if self.disabled then
      return false
   end
   if self.handler then
      self.handler(self)
      return true
   end
   return false
end
--[[--
Enables a button widget.
--]]
function luatk.Button:enable()
   self.disabled = false
end
--[[--
Disables a button widget.
--]]
function luatk.Button:disable()
   self.disabled = true
end
--[[--
Sets the alt text of a button.
--]]
function luatk.Button:setAlt( alt )
   self.alt = alt
end
function luatk.Button:getCol () return self.col end
function luatk.Button:setCol( col ) self.col = col end
function luatk.Button:getFCol () return self.fcol end
function luatk.Button:setFCol( col ) self.fcol = col end

--[[
-- Text widget
--]]
luatk.Text = {}
setmetatable( luatk.Text, { __index = luatk.Widget } )
luatk.Text_mt = { __index = luatk.Text }
--[[
Creates a new button widget.

   @tparam number x X position of the widget.
   @tparam number y Y position of the widget.
   @tparam number w Width of the widget.
   @tparam number h Height of the widget.
   @tparam string text Text to display on the widget.
--]]
function luatk.newText( parent, x, y, w, h, text, col, align, font )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Text_mt )
   wgt.text    = text
   wgt.col     = col or luatk.colour.text
   wgt.align   = align or "left"
   wgt.font    = font or luatk._deffont or lg.getFont()
   return wgt
end
function luatk.Text:draw( bx, by )
   if not self.text then return end
   lg.setColor( self.col )
   lg.printf( self.text, self.font, bx+self.x, by+self.y, self.w, self.align )
end
--[[--
Changes the text of the text widget.

   @tparam string text Text to set widget to.
--]]
function luatk.Text:set( text )
   self.text = text
end
--[[--
Gets the height of the text widget text.

   @treturn number Height of the text in the widget.
--]]
function luatk.Text:height ()
   local _w, h = self:dimensions()
   return h
end
--[[--
Gets the maximum width of the text widget text.

   @treturn number Width of the text in the widget.
--]]
function luatk.Text:width ()
   local w, _h = self:dimensions()
   return w
end
--[[--
Gets the dimensions of the text widget text.

   @treturn number Width of the text in the widget.
   @treturn number Height of the text in the widget.
--]]
function luatk.Text:dimensions ()
   local maxw, wrap = self.font:getWrap( self.text, self.w )
   return maxw, self.font:getLineHeight() * #wrap
end

--[[
-- Rectangle widget
--]]
luatk.Rect = {}
setmetatable( luatk.Rect, { __index = luatk.Widget } )
luatk.Rect_mt = { __index = luatk.Rect }
function luatk.newRect( parent, x, y, w, h, col, rot )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Rect_mt )
   wgt.col     = col or {1,1,1}
   wgt.rot     = rot
   return wgt
end
function luatk.Rect:draw( bx, by )
   lg.setColor( self.col )
   if self.rot then
      lg.push()
      -- TODO this is still off...
      lg.translate( bx+self.x, by+self.y )
      lg.rotate( self.rot )
      lg.translate( -self.w/2, -self.h/2 )
      lg.rectangle( "fill", 0, 0, self.w, self.h )
      lg.pop()
      return
   end
   lg.rectangle( "fill", bx+self.x, by+self.y, self.w, self.h )
end

--[[
-- Image widget
--]]
luatk.Image = {}
setmetatable( luatk.Image, { __index = luatk.Widget } )
luatk.Image_mt = { __index = luatk.Image }
function luatk.newImage( parent, x, y, w, h, img, col, rot )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Image_mt )
   wgt.img     = img
   wgt.col     = col or {1,1,1}
   wgt.rot     = rot or 0
   local iw, ih = wgt.img:getDimensions()
   wgt.w       = wgt.w / iw
   wgt.h       = wgt.h / ih
   return wgt
end
function luatk.Image:draw( bx, by )
   lg.setColor( self.col )
   self.img:draw( bx+self.x, by+self.y, self.rot, self.w, self.h )
end

--[[
-- Checkbox widget
--]]
luatk.Checkbox = {}
setmetatable( luatk.Checkbox, { __index = luatk.Widget } )
luatk.Checkbox_mt = { __index = luatk.Checkbox }
function luatk.newCheckbox( parent, x, y, w, h, text, handler, default )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Checkbox_mt )
   wgt.text    = text
   wgt.handler = handler
   wgt.state   = (default==true)
   wgt.font    = luatk._deffont or lg.getFont()
   wgt.fonth   = wgt.font:getHeight()
   return wgt
end
function luatk.Checkbox:draw( bx, by )
   bx = bx + self.x
   by = by + self.y
   local w, h = self.w, self.h

   lg.setColor( luatk.colour.dark )
   local s = 12
   lg.rectangle( "fill", bx, by+(h-s)*0.5, s, s )
   lg.setColor( luatk.colour.outline )
   s = 10
   lg.rectangle( "fill", bx+1, by+(h-s)*0.5, s, s )
   if self.state then
      lg.setColor( luatk.colour.dark )
      s = 6
      lg.rectangle( "fill", bx+3, by+(h-s)*0.5, s, s )
   end
   lg.setColor( luatk.colour.text )
   lg.printf( self.text, self.font, bx+15, by+(h-self.fonth)*0.5, w-15 )
end
function luatk.Checkbox:clicked()
   self.state = not self.state
   if self.handler then
      self.handler(self)
   end
end
function luatk.Checkbox:get() return self.state end
function luatk.Checkbox:set( state ) self.state = state end

--[[
-- Fader widget
--]]
luatk.Fader = {}
setmetatable( luatk.Fader, { __index = luatk.Widget } )
luatk.Fader_mt = { __index = luatk.Fader }
function luatk.newFader( parent, x, y, w, h, min, max, def, handler, params )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Fader_mt )
   params = params or {}
   wgt.handler = handler or function () end
   wgt.min = min
   wgt.max = max
   def = def or (min+max)*0.5
   wgt.val = math.min( math.max( def, min ), max )
   wgt.params = params
   wgt.font = params.font or luatk._deffont or lg.getFont()
   return wgt
end
function luatk.Fader:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h
   if self.params.labels then
      h = h-15
   end
   local off = (self.val-self.min)/(self.max-self.min)
   local cx = x + off * w
   local cy = y + h*0.5

   -- Track
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "fill", x, cy-2, w, 5 )

   -- Knob
   lg.setColor( luatk.colour.dark )
   lg.rectangle( "fill", cx-8, y-1, 17, h+2 )
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "fill", cx-7, y, 15, h )

   -- Labels
   if self.params.labels then
      local ly = y + h + 5
      lg.setColor( luatk.scrollbar.colour.label )
      if off * w > 40 then
         lg.printf( tostring(self.min), self.font, x-30, ly, 60, "center" )
      end
      if off * w < w-40 then
         lg.printf( tostring(self.max), self.font, x+w-30, ly, 60, "center" )
      end
      lg.setColor( luatk.colour.text )
      lg.setColor( luatk.scrollbar.colour.value )
      lg.printf( tostring(math.floor(self.val+0.5)), self.font, cx-30, ly, 60, "center" )
   end
end
function luatk.Fader:pressed( mx, _my )
   self:set( self.min + (mx / self.w) * self.max )
end
function luatk.Fader:mmoved( mx, my )
   if self._pressed then
      self:pressed( mx, my )
   end
end
function luatk.Fader:get()
   return self.val
end
function luatk.Fader:set( val, no_handler )
   self.val = math.max( self.min, math.min( self.max, val ) )
   if not no_handler then
      self.handler( self, self.val )
   end
end

--[[
-- List widget
--]]
luatk.List = {}
setmetatable( luatk.List, { __index = luatk.Widget } )
luatk.List_mt = { __index = luatk.List }
function luatk.newList( parent, x, y, w, h, items, onselect, defitem )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.List_mt )
   wgt.items   = items
   wgt.onselect= onselect or function () end
   wgt.selected= defitem or 1
   local font  = luatk._deffont or lg.getFont()
   wgt.cellh   = font:getLineHeight()
   wgt.cellpad = wgt.cellh - font:getHeight()
   wgt.maxh    = wgt.cellh * #items
   wgt.pos     = 0
   wgt.scrolls = wgt.maxh > wgt.h
   if wgt.scrolls then
      wgt.scrollh = wgt.maxh - wgt.h
   end
   return wgt
end
function luatk.List:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   -- Background
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "fill", x-2, y-2, w+4, h+4 )
   lg.setColor( luatk.colour.dark )
   lg.rectangle( "fill", x, y, w, h )

   -- Draw scrollbar
   local scs
   if self.scrolls then
      local scroll_pos = self.pos / self.scrollh
      w = w - 12
      drawScrollbar( x+w, y, 12, h, scroll_pos )

      -- Have to scissors here
      scs = lg.getScissor()
      lg.setScissor( x, y, w, h )
   end

   -- Draw selected item background
   local ty = y - self.pos + self.cellpad*0.5 + (self.selected-1) * self.cellh
   lg.setColor( luatk.colour.selected )
   lg.rectangle( "fill", x+1, ty, w-2, self.cellh )

   -- Draw content
   local font = luatk._deffont or lg.getFont()
   local xoff = x+6
   local yoff = y+4-self.pos
   local woff = w-4
   local hlim = by+self.y+self.h
   for k,v in ipairs( self.items ) do
      if yoff > -self.cellh then
         lg.setColor( luatk.colour.text )
         lg.printf( v, font, xoff, yoff, woff )
      end
      yoff = yoff + self.cellh

      -- Stop
      if yoff > hlim then break end
   end

   -- Undo scissors
   if self.scrolls then
      lg.setScissor( scs )
   end
end
function luatk.List:pressed( mx, my )
   if self.scrolls and mx > self.w-12 then
      self:setPos( (my-scrollbar_h*0.5) / (self.h-scrollbar_h) )
      return
   end
   self.selected = math.ceil( (my+self.pos-self.cellpad*0.5) / self.cellh )
   self.selected = math.max( 1, math.min( self.selected, #self.items ) )
   self.onselect( self:get() )
end
function luatk.List:mmoved( mx, my )
   if self._pressed then
      self:pressed( mx, my )
   end
end
function luatk.List:get()
   return self.items[ self.selected ], self.selected
end
function luatk.List:set( idx, no_handler )
   self.selected = math.max( 1, math.min( idx, #self.items ) )
   if not no_handler then
      self.onselect( self:get() )
   end
end
function luatk.List:setItem( itm, no_handler )
   for k,v in ipairs(self.items) do
      if v==itm then
         return self:set( k, no_handler )
      end
   end
end
function luatk.List:setPos( pos )
   self.pos = pos * self.scrollh
   self.pos = math.max( 0, math.min( self.scrollh, self.pos ) )
end

--[[
-- Input widget
--]]
luatk.Input = {}
setmetatable( luatk.Input, { __index = luatk.Widget } )
luatk.Input_mt = { __index = luatk.Input }
function luatk.newInput( parent, x, y, w, h, max, params )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Input_mt )
   params = params or {}
   wgt.max     = max
   wgt.params  = params
   wgt.font    = params.font or luatk._deffont or lg.getFont()
   wgt.fonth   = wgt.font:getHeight()
   wgt.fontlh  = wgt.font:getLineHeight()
   if (wgt.h-10) / wgt.font:getLineHeight() < 2 then
      wgt.oneline = true
   end
   wgt.whitelist = params.whitelist
   wgt.blacklist = params.blacklist
   wgt.str     = ""
   if params.str then
      wgt:set( params.str )
   end
   wgt.cursor = 0
   wgt.timer = 0
   return wgt
end
function luatk.Input:focus ()
   lk.setTextInput( true, self.parent.x+self.x, self.parent.y+self.y, self.w, self.h )
end
function luatk.Input:update( dt )
   self.timer = self.timer + dt
end
function luatk.Input:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   -- Background
   lg.setColor( luatk.colour.selected )
   lg.rectangle( "fill", x-2, y-2, w+4, h+4 )
   lg.setColor( luatk.colour.dark )
   lg.rectangle( "fill", x, y, w, h )

   lg.setColor( luatk.input.colour.text )
   local stry
   if self.oneline then
      stry = y+(h-self.fonth)*0.5
   else
      stry = y+5
   end
   lg.printf( self.str, self.font, x+5, stry, w-10 )

   if math.fmod( math.floor(self.timer*1.1), 2 )==0 then
      local fw = self.font:getWidth( utf8.sub( self.str, 1, self.cursor ) )
      if self.oneline then
         stry = y+(h-self.fontlh)*0.5
      else
         stry = y+5
      end
      lg.rectangle( "fill", x+5+fw+1, stry, 1, self.fontlh )
   end
end
function luatk.Input:get ()
   if not self.str or self.str=="" then
      return nil
   end
   return self.str
end
function luatk.Input:_addStr( str )
   local head = utf8.sub( self.str, 1, self.cursor )
   local tail = utf8.sub( self.str, self.cursor+1, utf8.len(self.str) )
   local body = ""
   for c in utf8.gmatch( str, "." ) do
      local doadd = true
      if self.whitelist and not self.whitelist[ c ] then
         doadd = false
      end
      if self.blacklist and self.blacklist[ c ] then
         doadd = false
      end
      if doadd then
         body = body..c
         self.cursor = self.cursor+1
      end
   end
   self.str = utf8.sub( head..body..tail, 1, self.max )
end
function luatk.Input:set( str )
   self.str = ""
   self:_addStr( str )
end
function luatk.Input:keypressed( key )
   if key == "backspace" then
      local l = utf8.len(self.str)
      if l > 0 then
         self.str = utf8.sub( self.str, 1, l-1 )
         self.cursor = math.max( 0, self.cursor-1 )
      end
      self.timer = 0
   elseif key == "left" then
      self.cursor = math.max( 0, self.cursor-1 )
      self.timer = 0
   elseif key == "right" then
      self.cursor = math.min( utf8.len(self.str), self.cursor+1 )
      self.timer = 0
   elseif key == "home" then
      self.cursor = 0
      self.timer = 0
   elseif key == "end" then
      self.cursor = utf8.len(self.str)
      self.timer = 0
   end
   return true
end
function luatk.Input:textinput( str )
   self.str = self.str or ""
   self:_addStr( str )
   return true
end

--[[
   High Level dialogue stuff
--]]
local function msgbox_size( title, msg )
   local font = luatk._deffont or lg.getFont()
   local titlelen = font:getWidth( title )
   local msglen = font:getWidth( msg )
   local wi = 10
   for i=0,11 do
      if msglen < (260 + i*50) * (2+i) then
         wi = i
         break
      end
   end
   local w = math.max( 300 + wi * 50, titlelen+40 )
   local _w, tw = font:getWrap( msg, w-40 )
   local h = #tw * font:getLineHeight()
   local d =  (w/h) * (3/4)
   if math.abs(d) > 0.3 then
      if h > w then
         w = h
      end
      _w, tw = font:getWrap( msg, w-40 )
      h = #tw * font:getLineHeight()
   end
   return w, h
end
function luatk.msg( title, msg )
   local w, h = msgbox_size( title, msg )

   local wdw = luatk.newWindow( nil, nil, w, 110 + h )
   local function wdw_done( dying_wdw )
      dying_wdw:destroy()
      return true
   end
   wdw:setAccept( wdw_done )
   wdw:setCancel( wdw_done )
   luatk.newText( wdw, 0, 10, w, 20, title, nil, "center" )
   luatk.newText( wdw, 20, 40, w-40, h, msg )
   luatk.newButton( wdw, (w-50)/2, h+110-20-30, 50, 30, _("OK"), function( wgt )
      wgt.parent:destroy()
   end )
   return wdw
end
function luatk.yesno( title, msg, funcyes, funcno )
   local w, h = msgbox_size( title, msg )
   local wdw = luatk.newWindow( nil, nil, w, 110 + h )
   luatk.newText( wdw, 0, 10, w, 20, title, nil, "center" )
   luatk.newText( wdw, 20, 40, w-40, h, msg )
   local bw = 120
   luatk.newButton( wdw, (w-bw)/2, h+110-20-30, 50, 30, _("Yes"), function( wgt )
      wgt.parent:destroy()
      if funcyes then
         funcyes()
      end
   end )
   luatk.newButton( wdw, (w-bw)/2+70, h+110-20-30, 50, 30, _("No"), function( wgt )
      wgt.parent:destroy()
      if funcno then
         funcno()
      end
   end )
   return wdw
end
function luatk.msgFader( title, msg, minval, maxval, def, funcdone )
   local w, h = msgbox_size( title, msg )

   local wdw = luatk.newWindow( nil, nil, w, 160 + h )
   luatk.newText( wdw, 0, 10, w, 20, title, nil, "center" )
   luatk.newText( wdw, 20, 40, w-40, h, msg )
   local fad = luatk.newFader( wdw, 20, h+105-20-30, w-40, 35, minval, maxval, def, nil, {
      labels = true,
   })
   local bw = 120
   local y = h+110-20-30+50
   luatk.newButton( wdw, (w-2*bw)/2-10, y, bw, 30, _("Accept"), function( wgt )
      wgt.parent:destroy()
      if funcdone then
         funcdone( fad:get() )
      end
   end )
   luatk.newButton( wdw, (w+0*bw)/2+10, y, bw, 30, _("Cancel"), function( wgt )
      wgt.parent:destroy()
      if funcdone then
         funcdone( nil )
      end
   end )
   local function wdw_done( dying_wdw, val )
      if funcdone then
         funcdone( val )
      end
      dying_wdw:destroy()
      return true
   end
   local function wdw_done_accept( dying_wdw )
      return wdw_done( dying_wdw, fad:get() )
   end
   local function wdw_done_cancel( dying_wdw )
      return wdw_done( dying_wdw, nil )
   end
   wdw:setAccept( wdw_done_accept )
   wdw:setCancel( wdw_done_cancel )
   return wdw
end
function luatk.msgInput( title, msg, max, funcdone, params )
   local w, h = msgbox_size( title, msg )

   local wdw = luatk.newWindow( nil, nil, w, 160 + h )
   luatk.newText( wdw, 0, 10, w, 20, title, nil, "center" )
   luatk.newText( wdw, 20, 40, w-40, h, msg )
   local inp = luatk.newInput( wdw, 20, h+100-20-20, w-40, 35, max, params )
   local bw = 120
   local y = h+110-20-30+50
   luatk.newButton( wdw, w-10-bw, y, bw, 30, _("Done"), function( wgt )
      wgt.parent:destroy()
      if funcdone then
         funcdone( inp:get() )
      end
   end )
   local function wdw_done( dying_wdw, val )
      if funcdone then
         funcdone( val )
      end
      dying_wdw:destroy()
      return true
   end
   local function wdw_done_accept( dying_wdw )
      return wdw_done( dying_wdw, inp:get() )
   end
   local function wdw_done_cancel( dying_wdw )
      return wdw_done( dying_wdw, nil )
   end
   wdw:setAccept( wdw_done_accept )
   wdw:setCancel( wdw_done_cancel )
   return wdw
end

function luatk.drawAltText( bx, by, alt, w )
   naev.gfx.clearDepth()
   local font = luatk._deffont or lg.getFont()
   w = w or 400
   local maxw, tw = font:getWrap( alt, w-20 )
   local h = #tw * font:getLineHeight() + 20
   local lw, lh = lg.getDimensions()
   w = math.min( maxw+20, w )

   local x = bx + 10
   local y = by
   if y+h+10 > lh then
      y = lh-h-10
   end
   if x+w+10 > lw then
      x = lw-w-10
   end

   lg.setColor( {0.0, 0.0, 0.0, 0.95} )
   lg.rectangle( "fill", x, y, w, h )
   lg.setColor( {0.5, 0.5, 0.5, 0.9} )
   lg.rectangle( "line", x, y, w, h )
   lg.setColor( luatk.colour.text )
   lg.printf( alt, font, x+10, y+10, w-20 )
end

return luatk
