local pixelcode_noise = love.filesystem.read( "noise.glsl" )
local pixelcode_nebula = love.filesystem.read( "nebula.frag" )
local pixelcode_wind = love.filesystem.read( "wind.frag" )
local pixelcode_digital = love.filesystem.read( "digital.frag" )
local pixelcode_marble = love.filesystem.read( "marble.frag" )
local pixelcode_electric = love.filesystem.read( "electric.frag" )
local pixelcode_starfield = love.filesystem.read( "starfiled.frag" )
local pixelcode_haze = love.filesystem.read( "haze.frag" )
local pixelcode_ionicstorm = love.filesystem.read( "ionicstorm.frag" )

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

function set_shader( num )
   shader_type = num
   --shader:send( "type", shader_type )
end

function love.load()
   local w = 800
   local h = 450
   love.window.setTitle( "Naev Overlay Demo" )
   love.window.setMode( w, h )
   --love.window.setMode( 0, 0, {fullscreen = true} )
   -- Set up the shader
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_nebula, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_wind, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_digital, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_marble, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_electric, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_starfield, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_haze, vertexcode)
   shader   = love.graphics.newShader( pixelcode_noise..pixelcode_ionicstorm, vertexcode)
   set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )

   if shader:hasUniform("u_resolution") then
      shader:send( "u_resolution", {w, h} )
   end
end

function love.keypressed(key)
   if key=="q" or key=="escape" then
      love.event.quit()
   end
end

function love.draw ()
   local lg = love.graphics

   local w, h = love.graphics.getDimensions()
   lg.setColor( 0, 0, 1, 1 )
   lg.setShader(shader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("u_time") then
      shader:send( "u_time", global_dt )
   end
end
