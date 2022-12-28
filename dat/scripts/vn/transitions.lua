--[[--
Transitions for the Visual Novel framework.
@submodule vn
--]]
local graphics = require 'love.graphics'
local love_math = require 'love.math'
local love_shaders = require 'love_shaders'

local transitions = {
   _t = {},
}

local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

transitions._t.fade = [[
vec4 effect( vec4 unused, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 c1 = Texel(texprev, texture_coords);
   vec4 c2 = Texel(tex, texture_coords);
   return mix( c1, c2, progress );
}
]]

transitions._t.blur = [[
#include "lib/blur.glsl"

const float INTENSITY = 3.0;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   float disp = INTENSITY*(0.5-distance(0.5, progress));
   vec4 c1 = blur9( texprev, uv, love_ScreenSize.xy, disp );
   vec4 c2 = blur9( tex, uv, love_ScreenSize.xy, disp );
   return mix(c1, c2, progress);
}
]]

transitions._t.circleopen = [[
#include "lib/math.glsl"
// Adapted from https://gl-transitions.com/editor/circleopen
// author: gre
// License: MIT

const float SMOOTHNESS = 0.3;
const vec2 CENTER = vec2(0.5, 0.5);

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   float x = progress;
   float m = smoothstep(-SMOOTHNESS, 0.0, M_SQRT2*distance(CENTER, uv) - x*(1.+SMOOTHNESS));
   vec4 c1 = Texel( texprev, uv );
   vec4 c2 = Texel( tex, uv );
   return mix( c1, c2, 1.0-m );
}
]]

transitions._t.circleclose = [[
#include "lib/math.glsl"
// Adapted from https://gl-transitions.com/editor/circleopen
// author: gre
// License: MIT

const float SMOOTHNESS = 0.3;
const vec2 CENTER = vec2(0.5, 0.5);

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   float x = 1.0-progress;
   float m = smoothstep(-SMOOTHNESS, 0.0, M_SQRT2*distance(CENTER, uv) - x*(1.+SMOOTHNESS));
   vec4 c1 = Texel( texprev, uv );
   vec4 c2 = Texel( tex, uv );
   return mix( c1, c2, m );
}
]]

transitions._t.dreamy = [[
// Adapted from https://gl-transitions.com/editor/Dreamy
// Author: mikolalysenko
// License: MIT

#include "lib/math.glsl"

vec2 offset( float progress, float x, float theta )
{
   float phase = progress*progress + progress + theta;
   float shifty = 0.03*progress*cos(10.0*(progress+x));
   return vec2(0.0, shifty);
}

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   vec4 c1 = Texel( texprev, uv + offset( progress, uv.x, 0.0 ) );
   vec4 c2 = Texel( tex, uv + offset( 1.0-progress, uv.x, M_PI ) );
   return mix( c1, c2, progress);
}
]]

transitions._t.ripple = [[
// Adapted from https://gl-transitions.com/editor/ripple
// Author: gre
// License: MIT

const float amplitude = 100.0;
const float speed = 50.0;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   vec2 dir = uv - vec2(0.5);
   float dist = length(dir);
   vec2 offset = dir * (sin(progress * dist * amplitude - progress * speed) + 0.5) / 30.0;
   vec4 c1 = Texel( texprev, uv + offset );
   vec4 c2 = Texel( tex, uv );
   return mix( c1, c2, smoothstep(0.2, 1.0, progress) );
}
]]

transitions._t.perlin = [[
#include "lib/perlin.glsl"

const float scale = 0.01;
const float smoothness = 0.1;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   float n = cnoise( scale * screen_coords + 1000.0*u_r )*0.5 + 0.5;
   float p = mix(-smoothness, 1.0 + smoothness, progress);
   float lower = p - smoothness;
   float higher = p + smoothness;
   float q = smoothstep(lower, higher, n);

   vec4 c1 = Texel( texprev, uv );
   vec4 c2 = Texel( tex, uv );
   return mix(c2, c1, q);
}
]]

transitions._t.wave = [[
// Adapted from https://gl-transitions._t.com/editor/ButterflyWaveScrawler
// Author: mandubian
// License: MIT

#include "lib/math.glsl"

const float amplitude = 1.0;
const float waves = 30.0;
const float colorSeparation = 0.3;

float compute(vec2 p, float progress, vec2 center) {
   vec2 o = p*sin(progress * amplitude)-center;
   // horizontal vector
   vec2 h = vec2(1.0, 0.0);
   // butterfly polar function (don't ask me why this one :))
   float theta = acos(dot(o, h)) * waves;
   return (exp(cos(theta)) - 2.0*cos(4.0*theta) + pow(sin((2.0*theta - M_PI) / 24.0), 5.0)) / 10.0;
}

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   vec2 p = uv / vec2(1.0);
   float inv = 1.0 - progress;
   vec2 dir = p - vec2(0.5);
   float dist = length(dir);
   float disp = compute(p, progress, vec2(0.5)) ;
   vec4 texTo = Texel( tex, p + inv*disp );

   float pdisp = progress*disp;
   vec4 texFromCenter = Texel( texprev, p + pdisp );
   vec4 texFrom = vec4(
         Texel( texprev, p + pdisp*(1.0 - colorSeparation)).r,
         texFromCenter.g,
         Texel( texprev, p + pdisp*(1.0 + colorSeparation)).b,
         texFromCenter.a );
   return texTo*progress + texFrom*inv;
}
]]

transitions._t.radial = [[
#include "lib/math.glsl"

const float smoothness = 1.0;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   vec2 rp = uv*2.0-1.0;
   vec4 c1 = Texel( tex, uv );
   vec4 c2 = Texel( texprev, uv );
   return mix( c1, c2,
         smoothstep(0.0, smoothness, atan(rp.y,rp.x) - (progress-0.5) * M_PI * 2.5)
         );
}
]]

transitions._t.pixelize = [[
// Adapted from https://gl-transitions.com/editor/pixelize
// Author: gre
// License: MIT
// forked from https://gist.github.com/benraziel/c528607361d90a072e98

#include "lib/math.glsl"

const ivec2 squaresMin = ivec2(20);
const int steps = 50;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords ) {
   float d = min(progress, 1.0 - progress);
   float dist = (steps>0) ? ceil(d * float(steps)) / float(steps) : d;
   vec2 squareSize = 2.0 * dist / vec2(squaresMin);
   vec2 p = (dist>0.0) ? (floor(uv / squareSize) + 0.5) * squareSize : uv;

   vec4 c1 = Texel( texprev, p );
   vec4 c2 = Texel( tex, p );
   return mix( c1, c2, progress);
}
]]

transitions._t.hexagon = [[
// Author: Fernando Kuteken
// License: MIT
// Hexagonal math from: http://www.redblobgames.com/grids/hexagons/

const int steps = 50;
uniform float horizontalHexagons = 20.0;
float ratio = love_ScreenSize.x / love_ScreenSize.y;

struct Hexagon {
   float q;
   float r;
   float s;
};

Hexagon createHexagon(float q, float r) {
   Hexagon hex;
   hex.q = q;
   hex.r = r;
   hex.s = -q - r;
   return hex;
}

Hexagon roundHexagon(Hexagon hex) {

   float q = floor(hex.q + 0.5);
   float r = floor(hex.r + 0.5);
   float s = floor(hex.s + 0.5);

   float deltaQ = abs(q - hex.q);
   float deltaR = abs(r - hex.r);
   float deltaS = abs(s - hex.s);

   if (deltaQ > deltaR && deltaQ > deltaS)
      q = -r - s;
   else if (deltaR > deltaS)
      r = -q - s;
   else
      s = -q - r;

   return createHexagon(q, r);
}

Hexagon hexagonFromPoint(vec2 point, float size) {

   point.y /= ratio;
   point = (point - 0.5) / size;

   float q = (sqrt(3.0) / 3.0) * point.x + (-1.0 / 3.0) * point.y;
   float r = 0.0 * point.x + 2.0 / 3.0 * point.y;

   Hexagon hex = createHexagon(q, r);
   return roundHexagon(hex);
}

vec2 pointFromHexagon(Hexagon hex, float size) {
   float x = (sqrt(3.0) * hex.q + (sqrt(3.0) / 2.0) * hex.r) * size + 0.5;
   float y = (0.0 * hex.q + (3.0 / 2.0) * hex.r) * size + 0.5;

   return vec2(x, y * ratio);
}

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords ) {

   float dist = 2.0 * min(progress, 1.0 - progress);
   dist = (steps > 0) ? ceil(dist * float(steps)) / float(steps) : dist;
   float size = (sqrt(3.0) / 3.0) * dist / horizontalHexagons;
   vec2 point = (dist > 0.0) ? pointFromHexagon(hexagonFromPoint(uv, size), size) : uv;

   vec4 c1 = Texel( texprev, point );
   vec4 c2 = Texel( tex, point );
   return mix( c1, c2, progress );
}
]]

transitions._t.crosshatch = [[
/* Loosely based on https://gl-transitions.com/editor/crosshatch
 * Author: pthrasher
 * License: MIT
 */
#include "lib/math.glsl"

const float THRESHOLD   = 3.0;
const float FADE_EDGE   = 0.1;

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords ) {
   vec4 c1 = Texel( texprev, uv );
   vec4 c2 = Texel( tex, uv );

   float dist = distance(vec2(0.5), uv) / THRESHOLD;
   float r = progress - min(random(vec2(uv.y, 0.0)), random(vec2(0.0, uv.x)));
   return mix( c1, c2, mix(0.0, mix(step(dist, r), 1.0, smoothstep(1.0-FADE_EDGE, 1.0, progress)), smoothstep(0.0, FADE_EDGE, progress)));
}
]]

transitions._t.burn = [[
uniform Image noisetex;

const float smoothness = 0.1;

vec4 burncolor( vec4 color, float value )
{
   const vec3 cred      = vec3( 1.00, 0.03, 0.00 );
   const vec3 corange   = vec3( 0.95, 0.3,  0.02 );
   const vec3 cblack    = vec3( 0.0 );
   const vec3 cwhite    = vec3( 1.0 );

   vec4 outcol = color;
   if (value <= 0.1)
      outcol.rgb = mix( cred, corange, value * (1.0/0.1) );
   else if (value < 0.4)
      outcol.rgb = mix( corange, cblack, value * 5.0 - 1.0 );
   else
      outcol.rgb *= mix( cblack, cwhite, (value-0.4)*(1.0/0.6) );
   return outcol;
}

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 px )
{
   float n = Texel( noisetex, uv ).r;
   float p = mix(-smoothness, 1.0 + smoothness, progress);
   float lower = p - smoothness;
   float higher = p + smoothness;
   float q = smoothstep(lower, higher, n);

   vec4 color = (q <= 0.0) ? Texel( tex, uv ) : burncolor( Texel( texprev, uv ), q );

   return color;
}
]]
local function _burn_noise ()
   local pixelcode = string.format([[
precision highp float;
#include "lib/simplex.glsl"
const float u_r = %f;
vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 px )
{
   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * 0.002 * m + 1000.0 * u_r ) * (1.0 / m);
   }
   return vec4( vec3(n)*0.5+0.5, 1.0 );
}
]], love_math.random() )
   local noiseshader = graphics.newShader( pixelcode, _vertexcode )
   return love_shaders.shader2canvas( noiseshader )
end

transitions._t.electric = [[
#include "lib/simplex.glsl"
uniform float u_time = 0.0;

const float height = 10.0;
//const vec3 bluetint = vec3( 0.132, 0.319, 1.0 );/* Gamma: vec3(0.4, 0.6, 0.8); */
const vec3 bluetint = vec3( 0.2, 0.5, 1.0 );/* Gamma: vec3(0.4, 0.6, 0.8); */

/* Similar to smoothbeam, but more k == sharper. */
float bump( float x )
{
   return min( cos( M_PI_2 * x ), 1.0 - abs(x) );
}

vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 px )
{
   float h = height / love_ScreenSize.y;
   float p = mix( -2.0*h, 1.0+2.0*h, progress );

   const float m = 1.0;
   float ybase, y, yoff, v;
   vec2 ncoord = vec2( 0.03 * px.x, 5.0*u_time ) + 100.0 * u_r;
   // Base arcs
   yoff = (1.0-p)*love_ScreenSize.y;
   ybase = yoff + height*snoise( ncoord );
   v = max( 0.0, bump( 0.3*distance(ybase,px.y) ) );

   // Extra arcs
   ncoord += vec2( 0.0, 5.0*u_time );
   y = yoff + height*snoise( 1.5*ncoord );
   v += max( 0.0, bump( 0.5*distance(y,px.y) ) );
   y = yoff + height*snoise( 2.0*ncoord );
   v += max( 0.0, bump( 0.5*distance(y,px.y) ) );

   // Create colour
   vec4 arcs = vec4( bluetint, v );

   // Reference alpha value
   vec2 uvr = vec2( uv.x, p );
   vec4 c1 = Texel( tex, uvr );
   vec4 c2 = Texel( texprev, uvr );

   // Sample texture
   vec4 colour;
   if (ybase < px.y)
      colour = Texel( tex, uv );
   else
      colour = Texel( texprev, uv );
   colour = mix( colour, arcs, v * max( c1.a, c2.a ));

   return colour;
}
]]

transitions._t.slideleft = [[
vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   uv -= vec2( 1.0-progress, 0.0 );
   return (uv.x < 0.0) ?
         Texel( texprev, uv + vec2(1.0, 0.0) ) :
         Texel( tex, uv );
}
]]

transitions._t.slideright = [[
vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   uv += vec2( 1.0-progress, 0.0 );
   return (uv.x > 1.0) ?
         Texel( texprev, uv - vec2(1.0, 0.0) ) :
         Texel( tex, uv );
}
]]

transitions._t.slidedown = [[
vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   uv += vec2( 0.0, 1.0-progress );
   return (uv.y > 1.0) ?
         Texel( texprev, uv - vec2(0.0, 1.0) ) :
         Texel( tex, uv );
}
]]

transitions._t.slideup = [[
vec4 effect( vec4 unused, Image tex, vec2 uv, vec2 screen_coords )
{
   uv -= vec2( 0.0, 1.0-progress );
   return (uv.y < 0.0) ?
         Texel( texprev, uv - vec2(0.0, 1.0) ) :
         Texel( tex, uv );
}
]]


function transitions.get( name, seconds, transition )
   -- Sane defaults
   name = name or "blur"
   transition = transition or "linear"
   if name=="blur" or name=="fade" then
      seconds = seconds or 0.2
   elseif name=="electric" then
      seconds = seconds or 1.0
   elseif name=="burn" or name=="ripple" or name=="perlin" or name=="wave" then
      seconds = seconds or 2.0
   else
      seconds = seconds or 1.0
   end

   local prefix = string.format( [[
uniform Image texprev;
uniform float progress = 0.0;
const float u_r = %f;
   ]], love_math.random() )

   local _pixelcode = prefix..transitions._t[name]
   if not _pixelcode then
      error( string.format(_("vn: unknown transition type'%s'"), name ) )
   end

   local shader = graphics.newShader( _pixelcode, _vertexcode )
   if name=="burn" then
      shader._noisetex = _burn_noise()
      shader:send( "noisetex", shader._noisetex )
   end
   if shader:hasUniform("u_time") then
      shader._dt = -100 * love_math.random()
      shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
   end
   return shader, seconds, transition
end

return transitions
