local pixelcode_sdf = love.filesystem.read( "frag.glsl" )
local _pixelcode = [[
#pragma language glsl3
uniform float u_time = 0.0;

vec4 bg( vec2 uv )
{
   vec3 c;
   uv *= 10.0;
   if (mod( floor(uv.x)+floor(uv.y), 2.0 ) == 0.0)
      c = vec3( 0.8 );
   else
      c = vec3( 0.2 );
   return vec4( c, 1.0 );
}

const float PULSE_SPEED    = 0.5;
const float PULSE_WIDTH    = 0.1;

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   //Sawtooth function to pulse from centre.
   float u_time = fract(u_time * PULSE_SPEED);
   vec2 off = uv - vec2(0.5);
   float dist = length( off );

   vec4 col;

   //Only distort the pixels within the parameter distance from the centre
   float diff = dist - u_time;
   if ((diff <= PULSE_WIDTH) && (diff >= -PULSE_WIDTH)) {
      //The pixel offset distance based on the input parameters
      //float sdiff = (1.0 - pow(abs(diff * 10.0), 0.38));
      float sdiff = (1.0 - abs(diff * 10.0));
      float tdist = u_time * dist;

      /* Perform the distortion and reduce the effect over time */
      uv += ((normalize(off) * diff * sdiff) / (tdist * 60.0));
      //Color = texture( tex, uv );
      col = bg( uv );

      /* Blow out the color and reduce the effect over time */
      col += color * sdiff / (tdist * 60.0);
   }
   else
      col = bg( uv );

   return col;
}
]]

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

local global_dt, img, shader

--local function set_shader( num )
   --shader_type = num
   --shader:send( "type", shader_type )
--end

function love.load()
   local ww, wh = 1200, 600
   love.window.setTitle( "Naev Overlay Demo" )
   love.window.setMode( ww, wh )
   --love.window.setMode( 0, 0, {fullscreen = true} )
   -- Set up the shader
   shader   = love.graphics.newShader( pixelcode_sdf, vertexcode)
   --set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 0.5, 0.5, 0.5, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
end

function love.keypressed(key)
   if key=="q" or key=="escape" then
      love.event.quit()
   end
end

function love.draw ()
   local lg = love.graphics
   local ww, wh = love.graphics.getDimensions()
   lg.setColor( 0, 0, 0, 1 )
   lg.rectangle( "fill", 0, 0, ww, wh )

   local x, y = 0, 0
   local parami = 0
   local function draw_shader( w )
      local h = w--/3
      shader:send("u_size",w/2)
      if shader:hasUniform("parami") then
         shader:send("parami", parami )
      end
      if shader:hasUniform("dimensions") then
         shader:send("dimensions", {w/2, w/2} )
      end
      y = (wh-h)/2.0
      lg.setShader()
      lg.setColor( 0.0, 0.0, 0.0, 1 )
      lg.rectangle( "fill", x, y, w, h )
      lg.setColor( 1, 1, 0, 0.5 )
      lg.setShader(shader)
      lg.draw( img, x, y, 0, w, h )

      x = x + w
      --parami =  --1 - parami
   end

   draw_shader( 600 )
   draw_shader( 300 )
   draw_shader( 150 )
   draw_shader(  75 )
   draw_shader(  38 )
   draw_shader(  20 )
   draw_shader(  10 )

   lg.setShader()
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("u_time") then
      shader:send( "u_time", global_dt )
   end
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end
