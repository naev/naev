--[[
-- Implementation of parts of the Naev toolkit... in Lua!
--]]
local lg = require 'love.graphics'
local le = require 'love.event'

local luatk = {
   _windows = {},

   colour = {
      bg       = { 0.2,  0.2,  0.2  },
      outline  = { 0.5,  0.5,  0.5  },
      dark     = { 0.05, 0.05, 0.05 },
      text     = { 0.95, 0.95, 0.95 },
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
   _deffont = nil,
}

--[[
-- Global functions
--]]
function luatk.run ()
   luatk._love = true
   love.exec( 'scripts/luatk' )  -- luacheck: ignore
   luatk._love = false
end
function luatk.setDefaultFont( font )
   luatk._deffont = font
end

--[[
-- Important functions
--]]
function luatk.draw()
   for _k,wdw in ipairs(luatk._windows) do
      wdw:draw()
   end
end
function luatk.update(dt)
   for _k,wdw in ipairs(luatk._windows) do
      wdw:update(dt)
   end
end
local function _checkbounds( b, mx, my )
   return not (mx < b.x or mx > b.x+b.w or my < b.y or my > b.y+b.h)
end
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
function luatk.mousemoved( mx, my )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw or not _checkbounds(wdw,mx,my) then return false end
   local x, y = mx-wdw.x, my-wdw.y

   for _k,wgt in ipairs(wdw._widgets) do
      local inbounds = _checkbounds( wgt, x, y )
      if not wgt._pressed then
         wgt.mouseover = inbounds
      end
      if inbounds and wgt.mmoved then
         wgt.mmoved( wgt, x-wgt.x, y-wgt.y )
      end
   end

   return false
end
function luatk.keypressed( key )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw then return false end

   if key=="return" then
      if wdw.accept and wdw:accept() then
         return true
      end
   elseif key=="escape" then
      if wdw.cancel and wdw:cancel() then
         return true
      end
   end

   return false
end

--[[
-- Window class
--]]
luatk.Window = {}
local Window_mt = { __index=luatk.Window }
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
end
function luatk.Window:destroy()
   for k,w in ipairs(luatk._windows) do
      if w==self then
         table.remove(luatk._windows,k)
         return
      end
   end
end
function luatk.close ()
   luatk._windows = {}
   if luatk._love then
      le.quit()
   end
end
function luatk.Window:setAccept( func )
   self.accept = func
end
function luatk.Window:setCancel( func )
   self.cancel = func
end

--[[
-- Widget class
--]]
luatk.Widget = {}
luatk.Widget_mt = { __index = luatk.Widget }
function luatk.newWidget( parent, x, y, w, h )
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
function luatk.Button:enable()
   self.disabled = false
end
function luatk.Button:disable()
   self.disabled = true
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
function luatk.newText( parent, x, y, w, h, text, col, align, font )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Text_mt )
   wgt.text    = text
   wgt.col     = col or luatk.colour.text
   wgt.align   = align or "left"
   wgt.font    = font or luatk._deffont or lg.newFont( 16 )
   return wgt
end
function luatk.Text:draw( bx, by )
   if not self.text then return end
   lg.setColor( self.col )
   lg.printf( self.text, self.font, bx+self.x, by+self.y, self.w, self.align )
end
function luatk.Text:set( text )
   self.text = text
end

--[[
-- Rectangle widget
--]]
luatk.Rect = {}
setmetatable( luatk.Rect, { __index = luatk.Widget } )
luatk.Rect_mt = { __index = luatk.Rect }
function luatk.newRect( parent, x, y, w, h, col )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Rect_mt )
   wgt.col     = col or {1,1,1}
   return wgt
end
function luatk.Rect:draw( bx, by )
   lg.setColor( self.col )
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
-- Fader widget
--]]
luatk.Fader = {}
setmetatable( luatk.Fader, { __index = luatk.Widget } )
luatk.Fader_mt = { __index = luatk.Fader }
function luatk.newFader( parent, x, y, w, h, min, max, def, handler )
   local wgt   = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, luatk.Fader_mt )
   wgt.handler = handler
   wgt.min = min
   wgt.max = max
   wgt.val = def
   return wgt
end
function luatk.Fader:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h
   local cx = x + (self.val-self.min)/(self.max-self.min) * w
   local cy = y + h*0.5

   -- Track
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "fill", x, cy-2, w, 5 )

   -- Knob
   lg.setColor( luatk.colour.dark )
   lg.rectangle( "fill", cx-8, y-1, 17, h+2 )
   lg.setColor( luatk.colour.outline )
   lg.rectangle( "fill", cx-7, y, 15, h )
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
function luatk.Fader:set( val )
   self.val = math.max( self.min, math.min( self.max, val ) )
   self.handler( self, self.val )
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
end

function luatk.drawAltText( bx, by, alt, w )
   naev.gfx.clearDepth()
   local font = luatk._deffont or lg.getFont()
   w = w or 250
   local _w, tw = font:getWrap( alt, w-20 )
   local h = #tw * font:getLineHeight() + 20
   local lw, lh = lg.getDimensions()

   local x = bx + 10
   local y = by
   if y+h+10 > lh then
      y = lh-h-10
   end
   if x+w+10 > lw then
      x = lw-w-10
   end

   lg.setColor( {0.0, 0.0, 0.0, 0.9} )
   lg.rectangle( "fill", x, y, w, h )
   lg.setColor( {0.5, 0.5, 0.5, 0.9} )
   lg.rectangle( "line", x, y, w, h )
   lg.setColor( luatk.colour.text )
   lg.printf( alt, font, x+10, y+10, w-20 )
end

return luatk
