--[[
-- Love2d Graphics for Naev!
--]]
love.graphics = {}
love.graphics._dx = 0
love.graphics._dy = 0
love.graphics._font = font.new( 12 )
love.graphics._bgcol = colour.new( 0, 0, 0, 1 )
love.graphics._fgcol = colour.new( 1, 1, 1, 1 )

-- Helper functions
local function _mode(m)
   if     m=="fill" then return false
   elseif m=="line" then return true
   else   error( string.format(_("Unknown fill mode '%s'"), mode ) )
   end
end
local function _xy( x, y, w, h )
   return love.x+love.graphics._dx+x, love.y+(love.h-y-h-love.graphics._dy)
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
   return colour.new( r, g, b, a or 1 )
end


--[[
-- Drawable class
--]]
love.graphics.Drawable = inheritsFrom( love.Object )
love.graphics.Drawable._type = "Drawable"
function love.graphics.Drawable:getDimensions() error('unimplemented') end


--[[
-- Image class
--]]
love.graphics.Image = inheritsFrom( love.graphics.Drawable )
love.graphics.Image._type = "Image"
function love.graphics.newImage( filename )
   local ttex
   if type(filename)=='string' then
      ttex = tex.open( love.filesystem.newFile( filename ) )
   elseif type(filename)=='table' and filename.type then
      local ot = filename:type()
      if ot=='ImageData' then
         ttex = tex.open( filename.data, filename.w, filename,h )
      end
   end
   if ttex ~= nil then
      local t = love.graphics.Image.new()
      t.tex = ttex
      return t
   end
   error(_('wrong parameter type'))
end
function love.graphics.Image:setFilter( min, mag ) end
function love.graphics.Image:getDimensions()
   local w,h = self.tex:dim()
   return w,h
end
function love.graphics.Image:getWidth()
   local w,h = self.tex:dim()
   return w
end
function love.graphics.Image:getHeight()
   local w,h = self.tex:dim()
   return h
end
function love.graphics.Image:_draw( ... )
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
      x = arg[5] or 1
      y = arg[6] or sx
      tx = q.x
      ty = q.y
      tw = q.w
      th = q.h
      x,y = _xy(x,y,w,h)
   end
   w = w*sx
   h = h*sy
   y = y - (h*(1-sy)) -- correct scaling
   gfx.renderTexRaw( self.tex, x, y, w, h, 1, 1, tx, ty, tw, th, love.graphics._fgcol, r )
end


--[[
-- Quad class
--]]
love.graphics.Quad = inheritsFrom( love.graphics.Drawable )
love.graphics.Quad._type = "Quad"
function love.graphics.newQuad( x, y, width, height, sw, sh )
   local q = love.graphics.Drawable.new()
   q.x = x/sw
   q.y = y/sw
   q.w = width/sw
   q.h = height/sh
   q.quad = true
   return q
end


--[[
-- SpriteBatch class
--]]
love.graphics.SpriteBatch = inheritsFrom( love.graphics.Drawable )
love.graphics.SpriteBatch._type = "SpriteBatch"
function love.graphics.newSpriteBatch( image, maxsprites, usage  )
   local batch = love.graphics.Drawable.new()
   return batch
end


--[[
-- Global functions
--]]
function love.graphics.getWidth()  return love.w end
function love.graphics.getHeight() return love.h end
function love.graphics.origin()
   love.graphics._dx = 0
   love.graphics._dy = 0
end
function love.graphics.translate( dx, dy )
   love.graphics._dx = love.graphics._dx + dx
   love.graphics._dy = love.graphics._dy + dy
end
function love.graphics.getBackgroundColor() return _gcol( self.graphics._bgcol ) end
function love.graphics.setBackgroundColor( red, green, blue, alpha )
   love.graphics._bgcol = _scol( red, green, blue, alpha )
end
function love.graphics.getColor() return _gcol( self.graphics._fgcol ) end
function love.graphics.setColor( red, green, blue, alpha )
   love.graphics._fgcol = _scol( red, green, blue, alpha )
end


--[[
-- Rendering primitives and drawing
--]]
function love.graphics.draw( drawable, ... )
   drawable:_draw( ... )
end
function love.graphics.rectangle( mode, x, y, width, height )
   x,y = _xy(x,y,width,height)
   gfx.renderRect( x, y, width, height, love.graphics._fgcol, _mode(mode) )
end
function love.graphics.circle( mode, x, y, radius )
   x,y = _xy(x,y,0,0)
   gfx.renderCircle( x, y, radius, love.graphics._fgcol, _mode(mode) )
end
function love.graphics.print( text, x, y  )
   x,y = _xy(x,y,limit,love.graphics._font:height())
   gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol )
end
function love.graphics.printf( text, x, y, limit, align )
   x,y = _xy(x,y,limit,love.graphics._font:height())
   if align=="left" then
      gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol, limit, false )
   elseif align=="center" then
      gfx.printf( love.graphics._font, text, x, y, love.graphics._fgcol, limit, true )
   elseif align=="right" then
      local w = gfx.printDim( false, text, limit )
      local off = limit-w
      gfx.printf( love.graphics._font, text, x+off, y, love.graphics._fgcol, w, false )
   end
end


--[[
-- Font stuff
--]]
function love.graphics.newFont( file, size )
   if size==nil then
      return font.new( file )
   elseif type(file)=="userdata" then
      return file
   else
      return font.new( file, size )
   end
end
function love.graphics.setFont( fnt )
   love.graphics._font = fnt
end
function love.graphics.setNewFont( file, size )
   love.graphics._font = love.graphics.newFont( file, size )
   return love.graphics._font
end


-- unimplemented
function love.graphics.setDefaultFilter( min, mag, anisotropy ) end
function love.graphics.setLineStyle( style ) end

return love.graphics
