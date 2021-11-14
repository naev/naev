--[[
   Some sort of stellar wind type background.
--]]
local bgshaders = require "bkg.bgshaders"
local love_shaders = require 'love_shaders'
local graphics = require "love.graphics"
local prng_lib = require "prng"
local prng = prng_lib.new()

local starfield = {}

starfield.stars = {
   "blue01.webp",
   "blue02.webp",
   "blue04.webp",
   "green01.webp",
   "green02.webp",
   "orange01.webp",
   "orange02.webp",
   "orange05.webp",
   "redgiant01.webp",
   "white01.webp",
   "white02.webp",
   "yellow01.webp",
   "yellow02.webp"
}

local starfield_frag = [[
#include "lib/gamma.glsl"
/*
 * Based on http://casual-effects.blogspot.com/2013/08/starfield-shader.html by Morgan McGuire
 * which is based on Star Nest by Kali https://www.shadertoy.com/view/XlfGRj
 * Both under MIT license.
 * Adapted to the Naev engine by bobbens
 */
uniform vec2 u_resolution;
uniform vec4 u_camera = vec4(1.0); /* xy corresponds to screen space */
uniform sampler2D u_prevtex;
const vec2 R      = vec2( %f, %f );
const float THETA = %f;
const mat2 ROT    = mat2( cos(THETA), -sin(THETA), sin(THETA), cos(THETA) );
const mat2 IROT   = inverse(ROT);

#define ITERATIONS   17
#define VOLSTEPS     8
#define SPARSITY     0.7  // 0.4 to 0.5 (sparse)
#define STEPSIZE     0.2
#define FREQVAR      1.8 // 0.5 to 2.0
#define BRIGHTNESS   0.0010
#define DISTFADING   0.6800

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = ROT*((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.w) + R;
   vec3 dir = vec3(uv, 1.0);
   vec3 cam = vec3(1.0) + vec3(ROT*u_camera.xy, u_camera.z);

   float s = 0.1, fade = 0.01;
   vec4 colour = vec4( vec3(0.0), 1.0 );

   for (int r=0; r < VOLSTEPS; r++) {
      vec3 p = cam + dir * (s * 0.5);
      p = abs(vec3(FREQVAR) - mod(p, vec3(FREQVAR * 2.0)));

      float prevlen = 0.0, a = 0.0;
      for (int i=0; i < ITERATIONS; i++) {
         p = abs(p);
         p = p * (1.0 / dot(p, p)) + (-SPARSITY); // the magic formula
         float len = length(p);
         a += abs(len - prevlen); // absolute sum of average change
         prevlen = len;
      }

      a *= a * a; // add contrast

      /* Colouring based on distance. */
      colour.rgb += (vec3(s, s*s, s*s*s) * a * BRIGHTNESS + 1.0) * fade;
      fade *= DISTFADING; /* Distance fading. */
      s += STEPSIZE;
   }
   colour.rgb = min(colour.rgb, vec3(1.2));

   /* Some cheap antialiasing filtering. */
   float intensity = min(colour.r + colour.g + colour.b, 0.7);
   float w = fwidth(intensity);
   colour.rgb = mix( colour.rgb, vec3(0.0), smoothstep(0.0,1.0,w) );

   /* Colour conversion. */
   colour.rgb = clamp( pow( colour.rgb, vec3(2.0) ), 0.0, 1.0 );

   /* Motion blur to increase temporal coherence and provide motion blur. */
   vec3 oldValue = texture(u_prevtex, texture_coords).rgb;
   colour.rgb = mix(oldValue - vec3(0.004), colour.rgb, 0.5);

   /* Darken it all a bit. */
   colour.rgb *= 0.8;
   return colour * colour_in;
}
]]

local shader, sstarfield, sf, sz, sb

local function star_add( added, num_added )
   -- Set up parameters
   local path  = "gfx/bkg/star/"
   -- Avoid repeating stars
   local stars = starfield.stars
   local cur_sys = system.cur()
   local num   = prng:random(1,#stars)
   local i     = 0
   while added[num] and i < 10 do
      num = prng:random(1,#stars)
      i   = i + 1
   end
   local star  = stars[ num ]
   -- Load and set stuff
   local img   = tex.open( path .. star )
   -- Position should depend on whether there's more than a star in the system
   local r     = prng:random() * cur_sys:radius()/3
   if num_added > 0 then
      r        = r + cur_sys:radius()*2/3
   end
   local a     = 2*math.pi*prng:random()
   local x     = r*math.cos(a)
   local y     = r*math.sin(a)
   local nmove = math.max( 0.05, prng:random()*0.1 )
   local move  = 0.02 + nmove
   local scale = 1.0 - (1 - nmove/0.2)/5
   bkg.image( img, x, y, move, scale ) -- On the background
   return num
end

local function add_local_stars ()
   -- Chose number to generate
   local n
   local r = prng:random()
   if r > 0.97 then
      n = 3
   elseif r > 0.94 then
      n = 2
   elseif r > 0.1 then
      n = 1
   end

   -- If there is an inhabited planet we'll need at least one star
   if not n then
      for _k,v in ipairs( system.cur():planets() ) do
         if v:services().land then
            n = 1
            break
         end
      end
   end

   -- Generate the stars
   local i = 0
   local added = {}
   while n and i < n do
      num = star_add( added, i )
      added[ num ] = true
      i = i + 1
   end
end

function starfield.init( params )
   params = params or {}

   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale * 0.5

   -- Per system parameters
   prng:setSeed( system.cur():nameRaw() )
   local theta = prng:random()*2*math.pi
   local rx, ry = vec2.newP( 6+1*prng:random(), 3+3*prng:random() ):get()
   sz = 1+1*prng:random()
   sb = naev.conf().bg_brightness

   -- Initialize shader
   shader = graphics.newShader( string.format(starfield_frag, rx, ry, theta), love_shaders.vertexcode )
   sstarfield = bgshaders.init( shader, sf, {usetex=true} )

   if not params.nolocalstars then
      add_local_stars()
   end
end

function starfield.render( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   x = x / 1e6
   y = y / 1e6
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, sz, z*0.0008*sf )

   sstarfield:render( dt, {1,1,1,sb} )
end

return starfield

