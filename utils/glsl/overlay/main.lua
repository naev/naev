--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3

uniform float dt;

vec4 vignette( vec4 color, vec2 uv, vec2 px )
{
   uv *= 1.0 - uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.25); // change pow for modifying the extend of the  vignette
   return vec4(vig); 
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out;
   vec2 pos = screen_coords;

   const vec2 size = vec2( 0.03, 0.03 );
   float total = floor(pos.x*float(size.x)) +
         floor(pos.y*float(size.y));
   bool isEven = mod(total,2.0)==0.0;
   vec4 col1 = vec4( 0.3, 0.3, 0.3, 1.0);
   vec4 col2 = vec4( 0.9, 0.9, 0.9, 1.0);
   color_out = (isEven)? col1:col2;

   vec4 val = vignette( color, texture_coords, screen_coords );
   color_out *= val;
   //color_out = mix( color_out, val, val.a );

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
end

function love.load()
   love.window.setTitle( "Naev Overlay Demo" )
   love.window.setMode( 0, 0, {fullscreen = true} )
   -- Set up the shader
   shader   = love.graphics.newShader(pixelcode, vertexcode)
   set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
end

function love.keypressed(key)
   local num = tonumber(key)
   if num~=nil then
      set_shader( num )
   elseif key=="q" or key=="escape" then
      love.event.quit()
   end
end

function love.draw ()
   local lg = love.graphics

   local w, h = love.graphics.getDimensions()
   lg.setColor( 1, 1, 1, 1 )
   lg.setShader(shader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()

   local x = 20
   local y = 10
   lg.print( string.format("Use the number keys to change between shaders.\nCurrent Shader: %d", shader_type), x, y )
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end

