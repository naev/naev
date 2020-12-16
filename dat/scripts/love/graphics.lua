--[[
-- Love2d Graphics for Naev!
--]]
local class = require 'class'

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
local function _xy( x, y, w, h )
   return love.x+graphics._dx+x, love.y+(love.h-y-h-graphics._dy)
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
graphics.Drawable = class.inheritsFrom( love.Object )
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
      ttex = naev.tex.open( love.filesystem.newFile( filename ) )
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
      sx = arg[5] or 1
      sy = arg[6] or sx
      tx = q.x
      ty = q.y
      tw = q.w
      th = q.h
      x,y = _xy(x,y,w,h)
   end
   w = w*sx --* graphics._sx
   h = h*sy --* graphics._sy
   y = y - (h*(1-sy)) -- correct scaling
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
function graphics.origin()
   -- TODO this translation/scaling stuff has to be done properly using
   -- homography matrices. Probably should employ src/opengl_matrix.c
   graphics._dx = 0
   graphics._dy = 0
   graphics._sx = 1
   graphics._sy = 1
end
function graphics.translate( dx, dy )
   graphics._dx = graphics._dx + dx
   graphics._dy = graphics._dy + dy
end
function graphics.scale( sx, sy )
   sy = sy or sx
   graphics._sx = graphics._sx * sx
   graphics._sy = graphics._sy * sy
end
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
   x,y = _xy(x,y,width,height)
   naev.gfx.renderRect( x, y, width, height, graphics._fgcol, _mode(mode) )
end
function graphics.circle( mode, x, y, radius )
   x,y = _xy(x,y,0,0)
   naev.gfx.renderCircle( x, y, radius, graphics._fgcol, _mode(mode) )
end
function graphics.print( text, x, y  )
   x,y = _xy(x,y,limit,graphics._font.font:height())
   naev.gfx.printf( graphics._font.font, text, x, y, graphics._fgcol )
end
function graphics.printf( text, x, y, limit, align )
   x,y = _xy(x,y,limit,graphics._font.font:height())
   if align=="left" then
      naev.gfx.printf( graphics._font.font, text, x, y, graphics._fgcol, limit, false )
   elseif align=="center" then
      naev.gfx.printf( graphics._font.font, text, x, y, graphics._fgcol, limit, true )
   elseif align=="right" then
      local w = naev.gfx.printDim( false, text, limit )
      local off = limit-w
      naev.gfx.printf( graphics._font.font, text, x+off, y, graphics._fgcol, w, false )
   end
end


--[[
-- Font stuff
--]]
graphics.Font = class.inheritsFrom( love.Object )
graphics.Font._type = "Font"
function graphics.newFont( file, size )
   local f = graphics.Font.new()
   if size==nil then
      if type(file)=="string" then
         file = love.filesystem.newFile( file ):getFilename()
      end
      f.font = naev.font.new( file )
   elseif type(file)=="userdata" then
      return file
   else
      file = love.filesystem.newFile( file ):getFilename()
      f.font = naev.font.new( file, size )
   end
   return f
end
function graphics.setFont( fnt )
   graphics._font = fnt
end
function graphics.setNewFont( file, size )
   graphics._font = graphics.newFont( file, size )
   return graphics._font
end


--[[
-- Shader class
--]]
graphics.Shader = class.inheritsFrom( love.Object )
graphics.Shader._type = "Shader"
function graphics.newShader( code )
   love._unimplemented()
   return graphics.Shader.new()
end


--[[
-- Canvas class
--]]
graphics.Canvas = class.inheritsFrom( love.Object )
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
graphics.setNewFont( 12 )
graphics.origin()
graphics.setDefaultFilter( "linear", "linear", 1 )
graphics._wraph = "clamp"
graphics._wrapv = "clamp"
graphics._wrapd = "clamp"

return graphics
