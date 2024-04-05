--[[--
A module containing a diversity of Love2D shaders for use in Naev. These are
designed to be used with the different aspects of the VN framework.

In general all shaders have a "strength" parameter indicating the strength
of the effect. Furthermore, those that have a temporal component have a
"speed" parameter. These are all normalized such that 1 is the default
value. Temporal component can also be inverted by setting a negative value.
@module love_shaders
--]]
local graphics = require "love.graphics"
local love_math = require "love.math"
local love_image = require "love.image"
local love_file = require 'love.filesystem'

local love_shaders = {}

--[[--
Shader common parameter table.
@tfield number strength Strength of the effect normalized such that 1.0 is the default value.
@tfield number speed Speed of the effect normalized such that 1.0 is the default value. Negative values run the effect backwards. Only used for those shaders with temporal components.
@tfield Colour colour Colour component to be used. Should be in the form of {r, g, b} where r, g, and b are numbers.
@tfield number size Affects the size of the effect.
@table shaderparams
--]]

-- Tiny image for activating shaders
local idata = love_image.newImageData( 1, 1 )
idata:setPixel( 0, 0, 1, 1, 1, 1 )
love_shaders.img = graphics.newImage( idata )

--[[--
Default fragment code that doesn't do anything fancy.
--]]
local _pixelcode = [[
vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 texcolour = Texel(tex, texture_coords);
   return texcolour * colour;
}
]]
--[[--
Default vertex code that doesn't do anything fancy.
--]]
local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]
-- Make default shaders visible.
love_shaders.pixelcode = _pixelcode
love_shaders.vertexcode = _vertexcode

local function _shader2canvas( shader, image, w, h, sx, sy )
   w = w or image.w
   h = h or image.h
   sx = sx or 1
   sy = sy or sx
   -- Render to image
   local newcanvas = graphics.newCanvas( w, h )
   local oldcanvas = graphics.getCanvas()
   local oldshader = graphics.getShader()
   graphics.setCanvas( newcanvas )
   graphics.clear( 0, 0, 0, 0 )
   graphics.setShader( shader )
   graphics.setColour( 1, 1, 1, 1 )
   image:draw( 0, 0, 0, sx, sy )
   graphics.setShader( oldshader )
   graphics.setCanvas( oldcanvas )

   return newcanvas
end

--[[--
Renders a shader to a canvas.

@tparam Shader shader Shader to render.
@tparam[opt=love.w] number width Width of the canvas to create (or nil for fullscreen).
@tparam[opt=love.h] number height Height of the canvas to create (or nil for fullscreen).
@tparam[param=love_shaders.img] Image img Image to use in the shader.
@treturn Canvas Generated canvas.
--]]
function love_shaders.shader2canvas( shader, width, height, img )
   img = img or love_shaders.img
   local lw, lh = naev.gfx.dim()
   width = width or lw
   height = height or lh
   return _shader2canvas( shader, img, width, height, width, height )
end


--[[--
Renders an image with a shader to a canvas.

@tparam Shader shader Shader to user
@tparam Image image Image to render.
@tparam[opt=image.w] number width Width of the canvas to create.
@tparam[opt=image.h] number height Height of the canvas to create.
@tparam[opt=1] number sx Scale factor for width.
@tparam[opt=1] number sy Scale factor for height.
@treturn Canvas Generated canvas.
--]]
love_shaders.shaderimage2canvas = _shader2canvas


--[[--
Generates a paper-like image.

@tparam number width Width of the image to create.
@tparam number height Height of the image to create.
@tparam[opt=1] number sharpness How sharp to make the texture look.
@treturn Canvas A apper-like canvas image.
--]]
function love_shaders.paper( width, height, sharpness )
   sharpness = sharpness or 1
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/paper.frag" ),
         love_math.random(), sharpness )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   return love_shaders.shader2canvas( shader, width, height )
end


--[[--
Blur shader applied to an image.

@tparam Drawable image A drawable to blur.
@tparam[opt=5] number kernel_size The size of the kernel to use to blur. This
   is the number of pixels in the linear case or the standard deviation in the
   Gaussian case.
@tparam[opt="gaussian"] string blurtype Either "linear" or "gaussian".
--]]
function love_shaders.blur( image, kernel_size, blurtype )
   kernel_size = kernel_size or 5
   blurtype = blurtype or "gaussian"
   local w, h = image:getDimensions()
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/blur.frag" ),
         w, h, kernel_size, blurtype )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   -- Since the kernel is separable we need two passes, one for x and one for y
   shader:send( "blurvec", 1, 0 )
   local pass1 = _shader2canvas( shader, image, w, h )
   local mode, alphamode = graphics.getBlendMode()
   graphics.setBlendMode( "alpha", "premultiplied" )
   shader:send( "blurvec", 0, 1 )
   local pass2 = _shader2canvas( shader, pass1, w, h )
   graphics.setBlendMode( mode, alphamode )
   return pass2
end

--[[--
Creates an oldify effect, meant for full screen effects.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.oldify( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/oldify.frag" ),
         strength )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000. * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A hologram effect, mainly meant for VN characters.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.hologram( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/hologram.frag" ),
         strength )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A corruption effect applies a noisy pixelated effect.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.corruption( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/corruption.frag" ),
         strength )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A rolling steamy effect. Meant as/for backgrounds.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" and "speed" fields is used.
--]]
function love_shaders.steam( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/steam.frag" ),
         strength, speed, love_math.random() )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
An electronic circuit-board like shader. Meant as/for backgrounds.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" and "speed" fields is used.
--]]
function love_shaders.circuit( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/circuit.frag" ),
         strength, speed, love_math.random() )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A windy type shader. Meant as/for backgrounds, however, it is highly transparent.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength", "speed", and "density" fields is used.
--]]
function love_shaders.windy( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local density = params.density or 1.0
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/windy.frag" ),
      strength, speed, density, love_math.random() )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
An aura effect for characters.

The default size is 40 and refers to the standard deviation of the Gaussian blur being applied.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength", "speed", "colour", and "size" fields are used.
--]]
function love_shaders.aura( params )
   params = params or {}
   local colour = params.colour or {1, 0, 0}
   local strength = params.strength or 1
   local speed = params.speed or 1
   local size = params.size or 40 -- Gaussian blur sigma
   local pixelcode = string.format(love_file.read( "scripts/loev_shaders/windy.frag" ),
         colour[1], colour[2], colour[3], strength, speed, love_math.random() )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader.prerender = function( self, image )
      self._blurtex = love_shaders.blur( image, size )
      self:send( "blurtex", self._blurtex )
      self.prerender = nil -- Run once
   end
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
Simple colour modulation shader.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "colour" field is used.
--]]
function love_shaders.colour( params )
   local colour = params.colour or {1, 1, 1, 1}
   colour[4] = colour[4] or 1
   local pixelcode = string.format([[
const vec4 basecolour = vec4( %f, %f, %f, %f );
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = Texel(tex, uv);
   return basecolour * colour * texcolour;
}
]], colour[1], colour[2], colour[3], colour[4] )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   return shader
end


--[[--
Simple colour modulation shader.

Same as love_shaders.colour, except it has an additional uniform 'strength' that controls how the tint is applied.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "colour" field is used.
--]]
function love_shaders.tint( params )
   local colour = params.colour or {1, 1, 1, 1}
   colour[4] = colour[4] or 1
   local pixelcode = string.format([[
uniform float strength = 1.0;
const vec4 basecolour = vec4( %f, %f, %f, %f );
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = Texel(tex, uv);
   return mix( basecolour, vec4(1.0), strength) * colour * texcolour;
}
]], colour[1], colour[2], colour[3], colour[4] )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader:send( "strength", 1 )
   return shader
end


return love_shaders
