--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3
#define M_PI 3.141592502593994140625

uniform float dt;

/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x, float k )
{
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), k );
}

/* k is the sharpness, more k == sharper.
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float sharpbeam( float x, float k )
{
   return 1. - pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

float random (vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float trail_default( float t, float y )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   a *= smoothbeam( y, 3.*m );

   return a;
}

float trail_pulse( float t, float y )
{
   float a, m, v;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   a *= smoothbeam( y, 3.*m );

   v = smoothstep( 0., 0.5, 1-t );
   a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (t * 25 + dt * 3) ), v );

   return a;
}

float trail_wave( float t, float y )
{
   float a, m, p;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   p = 2*M_PI * (t*5 + dt * 0.5);
   y += 0.2 * smoothstep(0, 0.8, 1-t) * sin( p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

float trail_flame( float t, float y )
{
   float a, m, p;

   // Modulate alpha base on length
   a = fastdropoff( t, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2*M_PI * (t*5 + dt * 5);
   y += 0.2 * smoothstep(0, 0.8, 1-t) * sin( p ) * sin( 2.7*p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

float trail_nebula( float t, float y )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( t, 1 );

   // Modulate alpha based on dispersion
   m = impulse( t, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-t );
   a *= 1-sharpbeam( y, 3*m );
   a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-t );

   return a;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
	vec4 color_out = color;
	vec2 pos = texture_coords;
	pos.y = 2*pos.y-1;
	pos.x = 1-pos.x;
   float t = pos.x;
   float a;
   int type = 2;

   if (type==1)
      a = trail_pulse( t, pos.y );
   else if (type==2)
      a = trail_wave( t, pos.y );
   else if (type==3)
      a = trail_flame( t, pos.y );
   else if (type==4)
      a = trail_nebula( t, pos.y );
   else
      a = trail_default( t, pos.y );

   color_out.a *= a;

	return color_out;
}
]]
 
local vertexcode = [[
#pragma language glsl3

uniform float dt;

vec4 position( mat4 transform_projection, vec4 vertex_position )
{
	return transform_projection * vertex_position;
}
]]
 

function love.load()
	shader 	= love.graphics.newShader(pixelcode, vertexcode)
	local idata = love.image.newImageData( 1, 1 )
	idata:setPixel( 0, 0, 1, 1, 1, 1 )
	img 		= love.graphics.newImage( idata )
end

function love.keypressed(key)
   love.event.quit()
end

function love.draw ()
	local x = 20
	local y = 20
	for _,w in ipairs( {700, 500, 300, 100} ) do
		for _,h in ipairs( {30, 20, 10} ) do
			love.graphics.setShader()
			love.graphics.setColor( 1, 1, 1, 0.5 )
			love.graphics.rectangle( "line", x-2, y-2, w+4, h+4 )
			love.graphics.setShader(shader)
			love.graphics.setColor( 0, 1, 1, 0.7 )
			love.graphics.draw( img, x, y, 0, w, h)
			y = y + h + 20
		end
	end
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end

