--[[
-- Implementation of parts of the Naev toolkit... in Lua!
--]]
local lg = require 'love.graphics'


local luatk = {
   _windows = {},

   colour = {
      bg       = { 0.2,  0.2,  0.2  },
      outline  = { 0.5,  0.5,  0.5  },
      dark     = { 0.05, 0.05, 0.05 },
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
function luatk.setDefaultFont( font )
   luatk._deffont = font
end


--[[
-- Important functions
--]]
function luatk.draw()
   for k,wdw in ipairs(luatk._windows) do
      wdw:draw()
   end
end
function luatk.update(dt)
   for k,wdw in ipairs(luatk._windows) do
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

   for k,wgt in ipairs(wdw._widgets) do
      if wgt.clicked and _checkbounds(wgt,x,y) then
         if wgt.clicked( wgt ) then
            return true
         end
      end
   end

   return false
end
function luatk.mousemoved( mx, my )
   local wdw = luatk._windows[ #luatk._windows ]
   if not wdw or not _checkbounds(wdw,mx,my) then return false end
   local x, y = mx-wdw.x, my-wdw.y

   for k,wgt in ipairs(wdw._widgets) do
      wgt.mouseover = _checkbounds(wgt,x,y)
   end

   return false
end
function luatk.keypressed( key )
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

   -- Draw widgets ontop
   for k,wgt in ipairs(self._widgets) do
      wgt:draw( x, y )
   end

   -- Restore scissors
   lg.setScissor( scs )
end
function luatk.Window:update(dt)
   for k,wgt in ipairs(self._widgets) do
      if wgt.update then
         wgt:update(dt)
      end
   end
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
function luatk.Widget:draw( x, y )
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
   wgt.text    = text
   wgt.handler = handler

   local font = luatk._deffont or lg.getFont()
   local sw, wrapped = font:getWrap( text, w )
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
      if self.mouseover then
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
   lg.printf( self.text, font, x, y+(h-self.th)/2, w, 'center' )
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
   wgt.col     = col or {1,1,1}
   wgt.align   = align or "left"
   wgt.font    = font or lg.newFont( 16 )
   return wgt
end
function luatk.Text:draw( bx, by )
   lg.setColor( self.col )
   lg.printf( self.text, self.font, bx+self.x, by+self.y, self.w, self.align )
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

return luatk
