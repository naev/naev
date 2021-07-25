local love = require 'love'
local lg = require 'love.graphics'
local love_shaders = require 'love_shaders'

local effects = {}

local alert_bg_shader_frag = [[
uniform float u_time = 0.0;

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

float sdCircle( in vec2 p, in float r )
{
   return length(p)-r;
}

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   color.a *= sin(u_time*20.0) * 0.1 + 0.9;
  
   /* Base Alpha */
   float a = step( sin((px.x + px.y) * 0.3), 0.0);

   /* Signed Distance Function Exclamation Point */
   vec2 p = 2.0*uv-1.0;
   p.y *= -1.0;
   float d = min( sdCircle( p+vec2(0.0,0.65), 0.15), sdUnevenCapsuleY( p+vec2(0,0.15), 0.1, 0.25, 0.7 ));

   /* Add border and make center solid. */
   a *= step( 0.0, d-0.1 );
   a += step( d, 0.0 );

   color.a *= a;
   return color;
}
]]
function effects.alert( params, x, y, z )
   if not effects.__alert_bg_shader then
      effects.__alert_bg_shader = lg.newShader(
         alert_bg_shader_frag,
         love_shaders.vertexcode
      )
   end
   effects.__alert_bg_shader:send( "u_time", params.time )

   local col = params.col or {1, 1, 0, 0.5}
   local s = params.size / z
   local old_shader = lg.getShader()
   lg.setShader( effects.__alert_bg_shader )
   lg.setColor( col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end


return effects
