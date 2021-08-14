local pixelcode_sdf = [[
#pragma language glsl3
uniform float u_time = 0.0;
uniform float dt = 0.0;
uniform float u_size = 100.0;
uniform vec2 dimensions;

const float M_PI        = 3.14159265358979323846;  /* pi */

float cro(in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }

float sdUnevenCapsuleY( in vec2 p, in float ra, in float rb, in float h )
{
	p.x = abs(p.x);

   float b = (ra-rb)/h;
   vec2  c = vec2(sqrt(1.0-b*b),b);
   float k = cro(c,p);
   float m = dot(c,p);
   float n = dot(p,p);

        if( k < 0.0   ) return sqrt(n)               - ra;
   else if( k > c.x*h ) return sqrt(n+h*h-2.0*h*p.y) - rb;
                        return m                     - ra;
}

float sdUnevenCapsule( in vec2 p, in vec2 pa, in vec2 pb, in float ra, in float rb )
{
   p  -= pa;
   pb -= pa;
   float h = dot(pb,pb);
   vec2  q = vec2( dot(p,vec2(pb.y,-pb.x)), dot(p,pb) )/h;

   //-----------

   q.x = abs(q.x);

   float b = ra-rb;
   vec2  c = vec2(sqrt(h-b*b),b);

   float k = cro(c,q);
   float m = dot(c,q);
   float n = dot(q,q);

   if( k < 0.0 ) return sqrt(h*(n            )) - ra;
   else if( k > c.x ) return sqrt(h*(n+1.0-2.0*q.y)) - rb;
   return m                       - ra;
}


// sca is the sin/cos of the orientation
// scb is the sin/cos of the aperture
float sdArc( in vec2 p, in vec2 sca, in vec2 scb, in float ra, in float rb )
{
   p *= mat2(sca.x,sca.y,-sca.y,sca.x);
   p.x = abs(p.x);
   float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p);
   return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - rb;
}

float sdCircle( in vec2 p, in float r )
{
   return length(p)-r;
}

vec4 sdf_alarm( vec4 color, Image tex, vec2 uv, vec2 px )
{
   color.a *= sin(u_time*20.0) * 0.1 + 0.9;

   /* Base Alpha */
   float a = step( sin((px.x + px.y) * 0.3), 0.0);

   /* Signed Distance Function Exclamation Point */
   vec2 p = 2.0*uv-1.0;
   p.y *= -1.0;
   float dc = sdCircle( p, 1.0 );
   p *= 1.2;
   float d = min( sdCircle( p+vec2(0.0,0.65), 0.15), sdUnevenCapsuleY( p+vec2(0,0.15), 0.1, 0.25, 0.7 ));

   a *= step( 0.0, d-20.0/u_size );
   a += step( d, 0.0 );

   /* Second border. */
   a *= step( dc+15.0/u_size, 0.0 );
   a += step( -(dc+15.0/u_size), 0.0 );
   a *= step( dc, 0.0 );

   color.a *= a;
   return color;
}

vec4 sdf_target( vec4 color, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   uv = abs(uv);

   const float arclen = M_PI/10.0;

   float d = sdArc( uv,
         vec2(sin(M_PI*0.75),cos(M_PI*0.75)),
         vec2(sin(arclen),cos(arclen)),
         0.95, 0.03 );

   d = min( d, sdUnevenCapsule( uv, vec2(0.68), vec2(0.8), 0.07, 0.02) );
   d -= (1.0+sin(3.0*dt)) * 0.007;
   d = max( -sdCircle( uv-vec2(0.68), 0.04 ), d );

   color.a *= smoothstep( -m, m, -d );
   return color;
}


vec4 bg( vec2 uv )
{
	vec3 c;
   uv *= 10.0;
	if (mod( floor(uv.x)+floor(uv.y), 2.0 ) == 0.0)
		c = vec3( 0.2 );
   else
      c = vec3( 0.0 );
   return vec4( c, 1.0 );
}

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 col_out;
   vec2 uv_rel = uv*2.0-1.0;

   //col_out = sdf_alarm( color, tex, uv, px );
   col_out = sdf_target( color, uv_rel );

   return mix( bg(uv), col_out, col_out.a );
}
]]
local pixelcode = [[
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

function set_shader( num )
   shader_type = num
   --shader:send( "type", shader_type )
end

function love.load()
   ww, wh = 1200, 600
   love.window.setTitle( "Naev Overlay Demo" )
   love.window.setMode( ww, wh )
   --love.window.setMode( 0, 0, {fullscreen = true} )
   -- Set up the shader
   shader   = love.graphics.newShader( pixelcode_sdf, vertexcode)
   set_shader( 0 )
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
   local w, h = love.graphics.getDimensions()
   lg.setColor( 0, 0, 0, 1 )
   lg.rectangle( "fill", 0, 0, w, h )

   local x, y = 0, 0
   local function draw_shader( w )
      local h = w
      shader:send("u_size",w/2)
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
   end

   draw_shader( 600 )
   draw_shader( 300 )
   draw_shader( 150 )
   draw_shader(  75 )
   draw_shader(  38 )

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

