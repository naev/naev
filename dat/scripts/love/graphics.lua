--[[
-- Love2d Graphics for Naev!
--]]
local class = require 'class'
local love = require 'love'
local object = require 'love.object'
local filesystem = require 'love.filesystem'
local love_math = require 'love.math'

local graphics = {
   _bgcol = naev.colour.new( 0, 0, 0, 1 ),
   _fgcol = naev.colour.new( 1, 1, 1, 1 ),
   _wraph = "clamp",
   _wrapv = "clamp",
   _wrapd = "clamp",
}

-- Helper functions
local function _mode(m)
   if     m=="fill" then return false
   elseif m=="line" then return true
   else   error( string.format(_("Unknown fill mode '%s'"), m ) )
   end
end
local function _H( x, y, r, sx, sy )
   -- TODO don't do this for every drawing...
   local H
   if graphics._canvas then
      -- Rendering to canvas
      local cw = graphics._canvas.t.w
      local ch = graphics._canvas.t.h
      H = naev.transform.ortho( 0, cw, 0, ch, 1, -1 )
      local cs = 1/graphics._canvas.t.s
      H = H:scale( cs, cs )
   else
      -- Rendering to screen
      H = graphics._O
   end
   H = graphics._T[1].T * H
   if x then
      if r == 0 then
         H = H:translate(x,y)
            :scale( sx, -sy )
            :translate(0,-1)
      else
         local hw = sx/2
         local hh = sy/2
         H = H:translate(x+hw,y+hh)
            :rotate2d(r)
            :translate(-hw,-hh)
            :scale( sx, -sy )
            :translate(0,-1)
      end
   end
   return H
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
function graphics.Drawable.draw() love._unimplemented() end


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
   -- Assume Naev texture
   elseif type(filename)=='userdata' then
      ttex = filename
   end
   if ttex ~= nil then
      local t = graphics.Image.new()
      t.tex = ttex
      t.w, t.h = ttex:dim()
      -- Set defaults
      t.s = 1
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
function graphics.Image:getDimensions() return self.w*self.s, self.h*self.s end
function graphics.Image:getWidth() return self.w*self.s end
function graphics.Image:getHeight() return self.h*self.s end
function graphics.Image:getDPIScale() return 1/self.s end
function graphics.Image:draw( ... )
   local arg = {...}
   local w = self.w
   local h = self.h
   local x,y,r,sx,sy,TH
   if type(arg[1])=='number' then
      -- x, y, r, sx, sy
      x = arg[1]
      y = arg[2]
      r = arg[3] or 0
      sx = arg[4] or 1
      sy = arg[5] or sx
   else
      -- quad, x, y, r, sx, sy
      local q = arg[1]
      x = arg[2]
      y = arg[3]
      r = arg[4] or 0
      sx = arg[5] or 1
      sy = arg[6] or sx
      TH = q.H.T
      w  = w * q.w
      h  = h * q.h
   end
   -- TODO be less horribly inefficient
   local shader = graphics._shader or graphics._shader_default
   shader = shader.shader
   local s1, s2, s3, s4
   local canvas = graphics._canvas
   if canvas then
      s1 = canvas.w
      s2 = canvas.h
      s3 = -1.0
      s4 = love.h
   else
      s1 = love.w
      s2 = love.h
      s3 = 1.0
      s4 = 0.0
   end
   -- TODO properly solve this, what happens is it gets run before the window size gets set
   shader:sendRaw( "love_ScreenSize", s1, s2, s3, s4 )

   -- Get transformation and run
   local s = self.s
   local H = _H( x, y, r, w*sx*s, h*sy*s )
   naev.gfx.renderTexH( self.tex, shader, H, graphics._fgcol, TH );
end


--[[
-- Quad class
--]]
graphics.Quad = class.inheritsFrom( graphics.Drawable )
graphics.Quad._type = "Quad"
function graphics.newQuad( x, y, width, height, sw, sh )
   local q = graphics.Drawable.new()
   if type(sw)~="number" then
      local t = sw
      sw = t.w
      sh = t.h
   end
   q.x = x/sw
   q.y = y/sh
   q.w = width/sw
   q.h = height/sh
   q.quad = true
   local H = love_math.newTransform()
   H:translate( q.x, q.y ):scale( q.w, q.h )
   q.H = H
   return q
end


--[[
-- Line stuff
--]]
function graphics.line( ... )
   naev.gfx.renderLinesH( _H(), graphics._fgcol, ... )
end


--[[
-- Transformation class
--]]
function graphics.origin()
   local nw, nh = naev.gfx.dim()
   local nx = -love.x
   local ny = love.h+love.y-nh
   graphics._O = naev.transform.ortho( nx, nx+nw, ny+nh, ny, 1, -1 )
   graphics._T = { love_math.newTransform() }
end
function graphics.push()
   local t = graphics._T[1]
   table.insert( graphics._T, 1, t:clone() )
end
function graphics.pop()
   table.remove( graphics._T, 1 )
   if graphics._T[1] == nil then
      graphics._T[1] = love_math.newTransform()
   end
end
function graphics.translate( dx, dy ) graphics._T[1]:translate( dx, dy ) end
function graphics.scale( sx, sy ) graphics._T[1]:scale( sx, sy ) end
function graphics.rotate( angle ) graphics._T[1]:rotate( angle ) end


--[[
-- SpriteBatch class
--]]
graphics.SpriteBatch = class.inheritsFrom( graphics.Drawable )
graphics.SpriteBatch._type = "SpriteBatch"
function graphics.newSpriteBatch( image, _maxsprites, _usage  )
   love._unimplemented()
   local batch = graphics.SpriteBatch.new()
   batch.image = image
   batch:clear()
   return batch
end
function graphics.SpriteBatch.clear( _self )
   love._unimplemented()
end
function graphics.SpriteBatch.setColour( _self )
   love._unimplemented()
end
graphics.SpriteBatch.setColor =  graphics.SpriteBatch.setColour -- love2d actually uses US spelling
function graphics.SpriteBatch.add( _self, ... )
   local _arg = {...}
   love._unimplemented()
end
function graphics.SpriteBatch.draw( _self )
   love._unimplemented()
end


--[[
-- Global functions
--]]
function graphics.getDimensions() return love.w, love.h end
function graphics.getWidth()  return love.w end
function graphics.getHeight() return love.h end
function graphics.getBackgroundColour() return _gcol( graphics._bgcol ) end
function graphics.setBackgroundColour( red, green, blue, alpha )
   graphics._bgcol = _scol( red, green, blue, alpha )
end
function graphics.getColour() return _gcol( graphics._fgcol ) end
function graphics.setColour( red, green, blue, alpha )
   graphics._fgcol = _scol( red, green, blue, alpha )
end
graphics.setColor = graphics.setColour -- love2d actually uses US spelling
graphics.getColor = graphics.getColour
graphics.getBackgroundColor = graphics.getBackgroundColour
graphics.setBackgroundColor = graphics.setBackgroundColour
function graphics.setDefaultFilter( min, mag, anisotropy )
   graphics._minfilter = min
   graphics._magfilter = mag or min
   graphics._anisotropy = anisotropy or 1
end
function graphics.getDefaultFilter()
   return graphics._minfilter, graphics._magfilter, graphics._anisotropy
end
function graphics.setBlendMode( mode, alphamode )
   alphamode = alphamode or "alphamultiply"
   naev.gfx.setBlendMode( mode, alphamode )
   graphics._mode = mode
   graphics._alphamode = alphamode
end
function graphics.getBlendMode()
   return graphics._mode, graphics._alphamode
end
-- unimplemented
function graphics.setLineStyle( _style )
   love._unimplemented()
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
      local g = arg[1][2]
      local b = arg[1][3]
      local a = arg[1][4] or 1
      col = _scol( r, g, b, a )
   end
   if graphics._canvas then
      graphics._canvas.canvas:clear( col )
   else
      -- Minor optimization: just render when there is non-transparent colour
      if col:alpha()>0 then
         naev.gfx.renderRect( love.x, love.y, love.w, love.h, col )
      end
   end
end
function graphics.draw( drawable, ... )
   drawable:draw( ... )
end
function graphics.rectangle( mode, x, y, width, height, pw, ph, segments )
   local H = _H( x, y, 0, width, height )
   naev.gfx.renderRectH( H, graphics._fgcol, _mode(mode), pw or 0, ph or 0, segments or 0 )
end

function graphics.circle( mode, x, y, radius )
   local H = _H( x, y-radius, 0, radius, radius )
   naev.gfx.renderCircleH( H, graphics._fgcol, _mode(mode) )
end
function graphics.print( text, ... )
   local arg = {...}
   local t = type(arg[1])
   -- We have to specify limit so we just put a ridiculously large value
   local w = 1e6
   local align = "left"
   if t=="number" then
      local x = arg[1]
      local y = arg[2]
      graphics.printf( text, x, y, w, align )
   else
      local font = arg[1]
      local x = arg[2]
      local y = arg[3]
      graphics.printf( text, font, x, y, w, align )
   end
end
function graphics.printf( text, ... )
   local arg = {...}
   local x, y, limit, align, font, col

   if type(arg[1])=="number" then
      -- love.graphics.printf( text, x, y, limit, align )
      font = graphics._font
      x = arg[1]
      y = arg[2]
      limit = arg[3]
      align = arg[4]
   else
      -- love.graphics.printf( text, font, x, y, limit, align )
      font = arg[1]
      x = arg[2]
      y = arg[3]
      limit = arg[4]
      align = arg[5]
   end
   align = align or "left"
   col = graphics._fgcol

   local H = _H( x, y+font.height, 0, 1, 1 )
   local sx = graphics._T[1].T:get()[1][1] -- X scaling
   local wrapped = naev.gfx.printfWrap( font.font, text, limit/sx )

   local atype
   if align=="left" then
      atype = 1
   elseif align=="center" then
      atype = 2
   elseif align=="right" then
      atype = 3
   end
   naev.gfx.printRestoreClear()
   for k,v in ipairs(wrapped) do
      local tx
      if atype==1 then
         tx = 0
      elseif atype==2 then
         tx = (limit-v[2])/2
      elseif atype==3 then
         tx = (limit-v[2])
      end
      naev.gfx.printRestoreLast()

      local HH = H:translate( sx*tx, 0 )
      naev.gfx.printH( HH, font.font, v[1], col, font.outline )
      H = H:translate( 0, -font.lineheight );
   end
end
function graphics.setScissor( x, y, width, height )
   if x then
      y = y or 0
      width = width or love.w
      height = height or love.h

      if graphics._canvas == nil then
         y = love.h - y - height
      end
      naev.gfx.setScissor( love.x+x, love.y+y, width, height )
   else
      x = 0
      y = 0
      width = love.w
      height = love.h
      naev.gfx.setScissor()
   end
   graphics._scissor = {x, y, width, height}
end
function graphics.getScissor ()
   return table.unpack( graphics._scissor )
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
      filename = arg[1]
      --filename = filesystem.newFile( arg[1] ):getFilename() -- Trick to set path
      size = arg[2] or 12
   else
      -- newFont( size )
      filename = nil
      size = arg[1] or 12
   end

   local f = graphics.Font.new()
   f.font, f.filename, f.prefix = naev.font.new( filename, size )
   f.height= f.font:height()
   f.lineheight = f.height*1.5 -- Naev default
   f:setFilter( graphics._minfilter, graphics._magfilter )
   f:setOutline( 0 )
   return f
end
function graphics.Font:setFallbacks( ... )
   local arg = {...}
   for k,v in ipairs(arg) do
      if not self.font:addFallback( v.filename, v.prefix ) then
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
-- setOutline is a Naev extension!!
function graphics.Font:setOutline( size )
   self.outline = size
end
function graphics.Font:getOutline()
   return self.outline
end
function graphics.setFont( fnt ) graphics._font = fnt end
function graphics.getFont() return graphics._font end
function graphics.setNewFont( file, size, ...  )
   local font = graphics.newFont( file, size, ... )
   graphics.setFont( font )
   return font
end


--[[
-- Shader class
--]]
-- Set some sane defaults.
local _pixelcode = [[
vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 texcolour = texture(tex, texture_coords );
   return texcolour * colour;
}
]]
local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]
graphics.Shader = class.inheritsFrom( object.Object )
graphics.Shader._type = "Shader"
function graphics.newShader( pixelcode, vertexcode )
   pixelcode = pixelcode or _pixelcode
   vertexcode = vertexcode or _vertexcode

   local prepend = [[
#version 150
#define _LOVE
// Syntax sugar
#define Image           sampler2D
#define ArrayImage      sampler2DArray
#define VolumeImage     sampler3D
#define Texel           texture
#define love_PixelColor colour_out
#define love_PixelColour colour_out
#define love_Position   gl_Position
#define love_PixelCoord love_getPixelCoord()

// Uniforms shared by pixel and vertex shaders
uniform mat4 ViewSpaceFromLocal;
uniform mat4 ClipSpaceFromView;
uniform mat4 ClipSpaceFromLocal;
uniform mat3 ViewNormalFromLocal;
uniform vec4 love_ScreenSize;
uniform vec4 ConstantColour = vec4(1.0);

// Compatibility
#define TransformMatrix             ViewSpaceFromLocal
#define ProjectionMatrix            ClipSpaceFromView
#define TransformProjectionMatrix   ClipSpaceFromLocal
#define NormalMatrix                ViewNormalFromLocal
]]
   local frag = [[
#define PIXEL
uniform sampler2D MainTex;

in vec4 VaryingTexCoord;
in vec4 VaryingColour;
in vec2 VaryingPosition;
out vec4 colour_out;

vec2 love_getPixelCoord() {
   vec2 uv = love_ScreenSize.xy * (0.5*VaryingPosition+0.5);
   uv.y = uv.y * love_ScreenSize.z + love_ScreenSize.w;
   return uv;
}

vec4 effect( vec4 vcolour, Image tex, vec2 texcoord, vec2 pixcoord );

void main(void) {
   love_PixelColour = effect( VaryingColour, MainTex, VaryingTexCoord.st, love_PixelCoord );
}
]]
   local vert = [[
#define VERTEX
in vec4 VertexPosition;
in vec4 VertexTexCoord;
in vec4 VertexColour;

out vec4 VaryingTexCoord;
out vec4 VaryingColour;
out vec2 VaryingPosition;

vec4 position( mat4 clipSpaceFromLocal, vec4 localPosition );

void main(void) {
    VaryingTexCoord  = VertexTexCoord;
    VaryingTexCoord.y= 1.0 - VaryingTexCoord.y;
    VaryingTexCoord  = ViewSpaceFromLocal * VaryingTexCoord;
    VaryingColour     = ConstantColour;
    love_Position    = position( ClipSpaceFromLocal, VertexPosition );
    VaryingPosition  = love_Position.xy;
}
]]
   local s = graphics.Shader.new()
   vertexcode = vertexcode or pixelcode
   s.shader = naev.shader.new(
         prepend..frag..pixelcode,
         prepend..vert..vertexcode )
   -- Set some default uniform values for when post-process shaders are used
   s.shader:sendRaw( "love_ScreenSize", love.w, love.h, 1.0, 0.0 )
   return s
end
function graphics.setShader( shader )
   graphics._shader = shader
end
function graphics.getShader()
   return graphics._shader
end
function graphics.Shader:send( name, ... )
   local arg = {...}
   if type(arg[1])=="table" then
      if arg[1]._type=="Image" then
         self.shader:send( name, arg[1].tex )
      elseif arg[1]._type=="Canvas" then
         self.shader:send( name, arg[1].t.tex )
      else
         self.shader:send( name, ... )
      end
   else
      self.shader:send( name, ... )
   end
end
function graphics.Shader:sendColour( name, col )
   -- Convert to naev colour so it does gamma conversion and return
   local c = naev.colour.new( table.unpack(col) )
   local t = { c:rgb() }
   t[4] = c:alpha()
   self.shader:send( name, t )
end
function graphics.Shader:hasUniform( name )
   return self.shader:hasUniform( name )
end


--[[
-- Canvas class
--]]
graphics.Canvas = class.inheritsFrom( object.Drawable )
graphics.Canvas._type = "Canvas"
function graphics.newCanvas( width, height, settings )
   settings = settings or {}
   local c = graphics.Canvas.new()
   if type(width)=='userdata' then -- assume Naev canvas
      c.canvas = width
      c.w, c.h = width:dims()
      c.s = 1
   else
      local nw, nh = naev.gfx.dim()
      c.w = width or nw
      c.h = height or nh
      local dpiscale = settings.dpiscale or graphics.getDPIScale()
      c.canvas = naev.canvas.new( c.w*dpiscale, c.h*dpiscale )
      c.s = 1/dpiscale
   end
   -- Set texture
   local t = graphics.Image.new()
   t.tex = c.canvas:getTex()
   t.w, t.h = t.tex:dim()
   t.s = c.s
   t:setFilter( graphics._minfilter, graphics._magfilter )
   t:setWrap( graphics._wraph, graphics._wrapv, graphics._wrapd )
   c.t = t
   return c
end
function graphics.setCanvas( canvas )
   if canvas==nil then
      naev.canvas.set()
   else
      naev.canvas.set( canvas.canvas )
   end
   graphics._canvas = canvas
end
function graphics.getCanvas()
   return graphics._canvas
end
function graphics.Canvas:draw(...)     return self.t:draw(...) end
function graphics.Canvas:setFilter(...)return self.t:setFilter(...) end
function graphics.Canvas:getFilter(...)return self.t:getFilter(...) end
function graphics.Canvas:setWrap(...)  return self.t:setWrap(...) end
function graphics.Canvas:getWrap(...)  return self.t:getWrap(...) end
function graphics.Canvas:getDimensions() return self.w, self.h end
function graphics.Canvas:getWidth()    return self.w end
function graphics.Canvas:getHeight()   return self.h end
function graphics.Canvas:getDPIScale() return 1/self.s end


--[[
   Misc
--]]
function graphics.isGammaCorrect() return true end
function graphics.isActive() return true end
function graphics.present() return end -- NOOP
function graphics.getDPIScale()
   local _w, _h, scale = naev.gfx.dim()
   return 1/scale
end
-- Have to initialize properly the dimensions here for when using the graphics
-- API outside of the love framework
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
graphics.setDefaultFilter( "linear", "linear", 1 )
graphics.setNewFont( 12 )
graphics.origin()
graphics._shader_default = graphics.newShader()
graphics.setShader( graphics._shader_default )
graphics.setCanvas( nil )
--graphics._mode = "alpha"
--graphics._alphamode = "alphamultiply"
graphics.setBlendMode( "alpha" )
graphics.setScissor()

return graphics
