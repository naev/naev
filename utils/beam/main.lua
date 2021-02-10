--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3
#define M_PI 3.141592502593994140625

#define BEAM_FUNC_PROTOTYPE

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;
uniform float r;

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

float beamfade( float x )
{
   if (x < .1) 
      return x / 0.1;
   else if (x > .8)
      return 1. - ((x - .8) / .2);
   return 1.;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p;

   color.a *= beamfade( pos_tex.x );

   p = 2*M_PI * (pos_px.x/20.- dt * 2. + r);
   y = pos_tex.y + 0.2 * sin( p );
   color.a *= smoothbeam( y, 3. );
   color.xyz *= 1. + 5*smoothbeam( y, 0.1 );

   return color;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out;
   vec2 pos = texture_coords;
   vec2 pos_px = pos * dimensions;
   pos.y = 2*pos.y-1;

   color_out = beam_default( color, pos, pos_px );

   return color_out;
}
]]

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

   shader_color = { 1, 0.3, 0.3, 0.7 }
end

function love.load()
   love.window.setTitle( "Naev Beam Demo" )
   -- Set up the shader
   shader   = love.graphics.newShader(pixelcode, vertexcode)
   set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
   -- Scaling
   scaling = 2

   -- Beams
   beams = {
      {
         h = 10,
         colour = { 1, 0, 0, 1 },
      }
   }
end

function love.keypressed(key)
   local num = tonumber(key)
   if num~=nil then
      set_shader( num )
   else
      love.event.quit()
   end
end

function love.draw ()
   local x = 20
   local y = 20
   local lg = love.graphics
   local s = scaling
   love.graphics.scale( s, s )
   y = y/s

   local w = 350
   for k,b in ipairs(beams) do
      h = b.h
      lg.setShader()
      lg.setColor( 1, 1, 1, 0.5 )
      lg.rectangle( "line", x-2, y-2, w+4, h+4 )
      lg.setShader(shader)
      shader:send( "r", 0 )
      shader:send( "dimensions", {w,h} )
      lg.setColor( shader_color )
      lg.draw( img, x, y, 0, w, h)
      y = y + h + 20/s
   end
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end

