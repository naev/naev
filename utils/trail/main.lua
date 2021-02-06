--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3
#define M_PI 3.141592502593994140625

/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x )
{
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), 1.0 );
   float s = step( x, 0.92 );
   float a = (0. - .63348*log(1.0 - x));
   float b = (-20.0 * x) + 20.0;
   return s*a + (1-s)*b;
}

/* k is the sharpness, more k == sharper.
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float random (vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
	vec4 color_out = color;
	vec2 pos = texture_coords;
	pos.y = 2*pos.y-1;
	pos.x = 1-pos.x;

	float m, d;
	float t = pos.x;

   // Modulate alpha base on length
   color_out.a *= fastdropoff( t );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-t, 30. );

   // Modulate width
   d = smoothbeam( pos.y, 3.*m );

   color_out.a *= d * (1. - (random(pos) * .3));

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


