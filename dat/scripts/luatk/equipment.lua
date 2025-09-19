--[[

   Equipment widget for Luatk Similar to in-game equipment window.

--]]
local luatk = require "luatk"
local lg = require "love.graphics"

local HEADER = 25 -- Space from top to top box
local BOX = 42 -- Size of outfit box (inner)
local BOXSEP = 14 -- Separation between inner outfit boxes
local COL = 60 -- Space between columns

local cOutfit = {
   Large  = colour.new("OutfitHeavy",  1, true),
   Medium = colour.new("OutfitMedium", 1, true),
   Small  = colour.new("OutfitLight",  1, true),
}
local function col_slotSize( s )
   return cOutfit[s]
end

local wgtEquipment = {}
setmetatable( wgtEquipment, { __index = luatk.Widget } )
local wgtEquipment_mt = { __index = wgtEquipment }
function wgtEquipment.new( parent, x, y, w, h, plt )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, wgtEquipment_mt )
   wgt.plt = plt

   local ps = plt:ship()
   wgt.oweap = { name=_("Weap.") }
   wgt.outil = { name=_("Util.") }
   wgt.ostru = { name=_("Stru.") }
   for k,v in ipairs(ps:getSlots()) do
      v.o = plt:outfitGet( k )
      -- Only add if meets criteria
      if not v.locked or v.visible or v.o~=nil then
         v.colsize = col_slotSize( v.size )
         if v.o ~= nil then
            v.img = lg.newImage( v.o:icon() )
            v.imgw, v.imgh = v.img:getDimensions()
         end
         if v.type=="Weapon" then
            table.insert( wgt.oweap, v )
         elseif v.type=="Utility" then
            table.insert( wgt.outil, v )
         elseif v.type=="Structure" then
            table.insert( wgt.ostru, v )
         end
      end
   end
   wgt.cols = { wgt.oweap, wgt.outil, wgt.ostru }

   return wgt
end

local function renderColumn( x, y, w, olist, title )
   lg.setColour( luatk.colour.text )
   lg.printf( title, x, y, w, "centre" )
   x = x + (w-BOX)*0.5
   y = y + HEADER
   for k,o in ipairs(olist) do
      local r,g,b = o.colsize:rgb()

      lg.setColour( r, g, b, 1 )
      lg.rectangle( "fill", x-3, y-3, BOX+6, BOX+6 )

      lg.setColour( r*0.6, g*0.6, b*0.6, 1 )
      lg.rectangle( "fill", x, y, BOX, BOX )

      if o.o ~= nil then
         lg.setColour( 1, 1, 1 )
         o.img:draw( x, y, 0, BOX/o.imgw, BOX/o.imgh )
      end

      -- TODO icons

      y = y+BOX+BOXSEP
   end
end
function wgtEquipment:draw( bx, by )
   local x, y = bx+self.x, by+self.y
   for k,c in ipairs(self.cols) do
      renderColumn( x, y, COL, c, c.name )
      x = x+COL
   end
end

function wgtEquipment:drawover( bx, by )
   if self.alt then
      luatk.drawAltText( bx+self.x+self.altx, by+self.y+self.alty, self.alt, 300 )
   end
end

function wgtEquipment:mmoved( mx, my, _dx, _dy )
   local ox, oy =  math.floor(mx / COL)+1, math.floor((my-HEADER+BOXSEP*0.5)/(BOX+BOXSEP))+1
   local c = self.cols[ox]
   local o = c and c[oy] or nil
   if not o or not o.o then
      self.alto = nil
      self.alt = nil
      luatk.rerender()
      return
   end
   if ox ~= self.altox or oy ~= self.altoy then
      self.alto  = o.o
      self.alt   = o.o:summary( self.plt )
      self.altx  = ox * COL
      self.alty  = oy * BOX+BOXSEP - BOXSEP*0.5
      self.altox = ox
      self.altoy = oy
      luatk.rerender()
   end
end

function wgtEquipment:dimensions()
   local w = COL * 3
   local h = HEADER + (BOX+BOXSEP)*math.max(#self.oweap,math.max(#self.outil,#self.ostru))
   return w, h
end
function wgtEquipment:height ()
   local _w, h = self:dimensions()
   return h
end
function wgtEquipment:width ()
   local w, _h = self:dimensions()
   return w
end

return wgtEquipment
