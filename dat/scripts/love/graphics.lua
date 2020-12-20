--[[
-- Love2d Graphics for Naev!
--]]
local class = require 'class'
local love = require 'love'
local object = require 'love.object'
local filesystem = require 'love.filesystem'
local love_math = require 'love.math'

local graphics = {}
graphics._bgcol = naev.colour.new( 0, 0, 0, 1 )
graphics._fgcol = naev.colour.new( 1, 1, 1, 1 )

-- Helper functions
local function _mode(m)
   if     m=="fill" then return false
   elseif m=="line" then return true
   else   error( string.format(_("Unknown fill mode '%s'"), mode ) )
   end
end
local function _xy( gx, gy, gw, gh )
   local x, y = graphics._T[1]:transformPoint( gx, gy )
   local w, h = graphics._T[1]:transformDim( gw, gh )
   return  love.x+x, love.y+(love.h-y-h), w, h
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
   return naev.colour.new( r, g, b, a or 1 )
end


--[[
-- Drawable class
--]]
graphics.Drawable = class.inheritsFrom( object.Object )
graphics.Drawable._type = "Drawable"
function graphics.Drawable._draw() error(_("unimplemented")) end


--[[
-- Image class
--]]
graphics.Image = class.inheritsFrom( graphics.Drawable )
graphics.Image._type = "Image"
function graphics.newImage( filename )
   local ttex
   if type(filename)=='string' then
      ttex = naev.tex.open( filesystem.newFile( filename ) )
   elseif type(filename)=='table' and filename.type then
      local ot = filename:type()
      if ot=='ImageData' then
         ttex = naev.tex.open( filename.d, filename.w, filename.h )
      end
   end
   if ttex ~= nil then
      local t = graphics.Image.new()
      t.tex = ttex
      t.w, t.h = ttex:dim()
      -- Set defaults
      t:setFilter( graphics._minfilter, graphics._magfilter )
      t:setWrap( graphics._wraph, graphics._wrapv, graphics._wrapd )
      return t
   end
   error(_('wrong parameter type'))
end
function graphics.Image:setFilter( min, mag, anisotropy )
   mag = mag or min
   anisotropy = anisotropy or 1
   self.tex:setFilter( min, mag, anisotropy )
   self.min = min
   self.mag = mag
   self.anisotropy = anisotropy
end
function graphics.Image:getFilter() return self.min, self.mag, self.anisotropy end
function graphics.Image:setWrap( horiz, vert, depth )
   vert = vert or horiz
   depth = depth or horiz
   self.tex:setWrap( horiz, vert, depth )
   self.wraph = horiz
   self.wrapv = vert
   self.wrapd = depth
end
function graphics.Image:getWrap() return self.wraph, self.wrapv, self.wrapd end
function graphics.Image:getDimensions() return self.w, self.h end
function graphics.Image:getWidth() return self.w end
function graphics.Image:getHeight() return self.h end
function graphics.Image:_draw( ... )
   local arg = {...}
   local w,h = self.tex:dim()
   local x,y,r,sx,sy,tx,ty,tw,th
   if type(arg[1])=='number' then
      -- x, y, r, sx, sy
      x = arg[1]
      y = arg[2]
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
      sx = arg[5] or 1
      sy = arg[6] or sx
      tx = q.x
      ty = q.y
      tw = q.w
      th = q.h
   end
   w = w*sx --* graphics._sx
   h = h*sy --* graphics._sy
   --y = y - (h*(1-sy)) -- correct scaling
   x,y,w,h = _xy(x,y,w,h)
   naev.gfx.renderTexRaw( self.tex, x, y, w*tw, h*th, 1, 1, tx, ty, tw, th, graphics._fgcol, r )
end


--[[
-- Quad class
--]]
graphics.Quad = class.inheritsFrom( graphics.Drawable )
graphics.Quad._type = "Quad"
function graphics.newQuad( x, y, width, height, sw, sh )
   local q = graphics.Drawable.new()
   q.x = x/sw
   q.y = y/sh
   q.w = width/sw
   q.h = height/sh
   q.quad = true
   return q
end


--[[
-- Transformation class
--]]
function graphics.origin()
   graphics._T = { love_math.newTransform() }
end
function graphics.push()
   local t = graphics._T[1]
   table.insert( graphics._T, 1, t:clone() )
end
function graphics.pop()
   table.remove( graphics._T, 1 )
end
function graphics.translate( dx, dy ) graphics._T[1]:translate( dx, dy ) end
function graphics.scale( sx, sy ) graphics._T[1]:scale( sx, sy ) end
function graphics.rotate( angle ) graphics._T[1]:rotate( angle ) end


--[[
-- SpriteBatch class
--]]
graphics.SpriteBatch = class.inheritsFrom( graphics.Drawable )
graphics.SpriteBatch._type = "SpriteBatch"
function graphics.newSpriteBatch( image, maxsprites, usage  )
   local batch = graphics.SpriteBatch.new()
   batch.image = image
   batch:clear()
   love._unimplemented()
   return batch
end
function graphics.SpriteBatch:clear()
   love._unimplemented()
end
function graphics.SpriteBatch:setColor()
   love._unimplemented()
end
function graphics.SpriteBatch:add( ... )
   local arg = {...}
   love._unimplemented()
end
function graphics.SpriteBatch:_draw()
   love._unimplemented()
end


--[[
-- Global functions
--]]
function graphics.getDimensions() return love.w, love.h end
function graphics.getWidth()  return love.w end
function graphics.getHeight() return love.h end
function graphics.getBackgroundColor() return _gcol( graphics._bgcol ) end
function graphics.setBackgroundColor( red, green, blue, alpha )
   graphics._bgcol = _scol( red, green, blue, alpha )
end
function graphics.getColor() return _gcol( graphics._fgcol ) end
function graphics.setColor( red, green, blue, alpha )
   graphics._fgcol = _scol( red, green, blue, alpha )
end
function graphics.setDefaultFilter( min, mag, anisotropy )
   graphics._minfilter = min
   graphics._magfilter = mag
   graphics._anisotropy = 1
end
function graphics.getDefaultFilter()
   return graphics._minfilter, graphics._magfilter, graphics._anisotropy
end


--[[
-- Rendering primitives and drawing
--]]
function graphics.clear( ... )
   local arg = {...}
   local col
   if #arg==0 then
      col = graphics._bgcol
   elseif type(arg[1])=="number" then
      local r = arg[1]
      local g = arg[2]
      local b = arg[3]
      local a = arg[4] or 1
      col = _scol( r, g, b, a )
   elseif type(arg[1])=="table" then
      local r = arg[1][1]
      local g = arg[1][1]
      local b = arg[1][1]
      local a = arg[1][1] or 1
      col = _scol( r, g, b, a )
   end
   naev.gfx.renderRect( love.x, love.y, love.w, love.h, col )
end
function graphics.draw( drawable, ... )
   drawable:_draw( ... )
end
function graphics.rectangle( mode, x, y, width, height )
   x,y,w,h = _xy(x,y,width,height)
   naev.gfx.renderRect( x, y, w, h, graphics._fgcol, _mode(mode) )
end
function graphics.circle( mode, x, y, radius )
   x,y = _xy(x,y,0,0)
   naev.gfx.renderCircle( x, y, radius, graphics._fgcol, _mode(mode) )
end
function graphics.print( text, x, y )
   -- We have to specify limit so we just put a ridiculously large value
   graphics.printf( text, x, y, 1e6, "left" )
end
function graphics.printf( text, ... )
   local arg = {...}
   local x, y, limit, align, font, col

   if type(arg[1])=="table" then
      -- love.graphics.printf( text, font, x, y, limit, align )
      font = arg[1]
      x = arg[2]
      y = arg[3]
      limit = arg[4]
      align = arg[5] or "left"
   else
      -- love.graphics.printf( text, x, y, limit, align )
      font = graphics._font
      x = arg[1]
      y = arg[2]
      limit = arg[3]
      align = arg[4] or "left"
   end
   col = graphics._fgcol

   x,y = _xy(x,y,limit,graphics._font.height)

   local wrapped, maxw = naev.gfx.printfWrap( font.font, text, limit )

   local atype
   if align=="left" then
      atype = 1
   elseif align=="center" then
      atype = 2
   elseif align=="right" then
      atype = 3
   end
   for k,v in ipairs(wrapped) do
      local tx
      if atype==1 then
         tx = x
      elseif atype==2 then
         tx = x + (limit-v[2])/2
      elseif atype==3 then
         tx = x + (limit-v[2])
      end
      naev.gfx.printf( font.font, v[1], tx, y, col )
      y = y - font.lineheight
   end
end


--[[
-- Font stuff
--]]
graphics.Font = class.inheritsFrom( object.Object )
graphics.Font._type = "Font"
function graphics.newFont( ... )
   local arg = {...}
   local filename, size
   if type(arg[1])=="string" then
      -- newFont( filename, size )
      filename = filesystem.newFile( arg[1] ):getFilename() -- Trick to set path
      size = arg[2] or 12
   else
      -- newFont( size )
      filename = nil
      size = arg[1] or 12
   end

   local f = graphics.Font.new()
   f.font = naev.font.new( filename, size )
   f.filename = filename
   f.height= f.font:height()
   f.lineheight = f.height*1.5 -- Naev default
   f:setFilter( graphics._minfilter, graphics._magfilter )
   return f
end
function graphics.Font:setFallbacks( ... )
   local arg = {...}
   for k,v in ipairs(arg) do
      local filename = v.filename
      print( filename )
      if not self.font:addFallback( filename ) then
         error(_("failed to set fallback font"))
      end
   end
end
function graphics.Font:getWrap( text, wraplimit )
   local wrapped, maxw = naev.gfx.printfWrap( self.font, text, wraplimit )
   local wrappedtext = {}
   for k,v in ipairs(wrapped) do
      wrappedtext[k] = v[1]
   end
   return maxw, wrappedtext
end
function graphics.Font:getHeight() return self.height end
function graphics.Font:getWidth( text ) return self.font:width( text ) end
function graphics.Font:getLineHeight() return self.lineheight end
function graphics.Font:setLineHeight( height ) self.lineheight = height end
function graphics.Font:getFilter() return self.min, self.mag, self.anisotropy end
function graphics.Font:setFilter( min, mag, anisotropy )
   mag = mag or min
   anisotropy = anisotropy or 1
   self.font:setFilter( min, mag, anisotropy )
   self.min = min
   self.mag = mag
   self.anisotropy = anisotropy
end
function graphics.setFont( fnt ) graphics._font = fnt end
function graphics.getFont() return graphics._font end
function graphics.setNewFont( file, size )
   local font = graphics.newFont( file, size )
   graphics.setFont( font )
   return font
end


--[[
-- Shader class
--]]
graphics.Shader = class.inheritsFrom( object.Object )
graphics.Shader._type = "Shader"
function graphics.newShader( code )
   love._unimplemented()
   return graphics.Shader.new()
end


--[[
-- Canvas class
--]]
graphics.Canvas = class.inheritsFrom( object.Object )
graphics.Canvas._type = "Canvas"
function graphics.newCanvas( width, height, settings )
   love._unimplemented()
   return graphics.Canvas.new()
end


-- unimplemented
function graphics.setLineStyle( style )
   love._unimplemented()
end
function graphics.setBlendMode( mode )
   love._unimplemented()
end


-- Reset coordinate system
graphics.setDefaultFilter( "linear", "linear", 1 )
graphics.setNewFont( 12 )
graphics.origin()
graphics._wraph = "clamp"
graphics._wrapv = "clamp"
graphics._wrapd = "clamp"

return graphics
