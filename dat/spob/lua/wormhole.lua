local lg = require "love.graphics"
local love_shaders = require "love_shaders"

local pixelcode = [[
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform float u_r = 0.0;

const vec3 col_inner  = vec3( 0.2, 0.8, 1.0 );
const vec3 col_outter = vec3( 0.0, 0.8, 1.0 );

float fbm3( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<3; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

float fbm5( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<5; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float t = u_time * 0.2;

   vec2 uv = texture_coords-0.5;
   vec2 st = vec2( 1.5 * length(uv), atan(uv.y,uv.x) );

   st.y += 1.1 * st.x;
   float x = fbm3( vec3( sin(st.y), cos(st.y), pow(st.x, 0.3) + 0.1*t ) ); // 3
   float y = fbm3( vec3( cos(1.0-st.y), sin(1.0-st.y), pow(st.x, 0.5) + 0.1*t ) ); // 4

   float r = fbm5( vec3( x, y, st.x + 0.3*t ) ); // 5
   r = fbm5( vec3( r-x, r-y, r + 0.3*t ) ); // 6

   float c = (r + 5.0*st.x) / 6.0;

   /* Determine the inner and outter alpha values. */
   float a_in  = 1.0-smoothstep( 0.2, 0.35, c );
   float a_out = 1.0-smoothstep( 0.35, 0.6, c );

   /* Set up the colours. */
   vec3 col = vec3(0.0);
   col = mix( col, col_outter, a_out );
   col = mix( col, col_inner,  a_in );

   /* Inner stuff. */
   col = mix( col, vec3(0.0), a_in-0.3 );

   return vec4( col, a_out );
}
]]

local cvs, shader, pos
local w, h = 256, 256

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   local oldshader = lg.getShader()
   lg.setShader( shader )
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColor( 1, 1, 1, 1 )
   lg.setBlendMode( "alpha", "premultiplied" )
   love_shaders.img:draw( 0, 0, 0, w, h )
   lg.setBlendMode( "alpha" )
   lg.setShader( oldshader )
   lg.setCanvas( oldcanvas )
end

function load( p )
   if shader==nil then
      shader = lg.newShader( pixelcode, love_shaders.vertexcode )
      shader._dt = -1000 * rnd.rnd()
      shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
      pos = p:pos()
      pos = pos + vec2.new( -w/2, h/2 )
      cvs = lg.newCanvas( w, h, {dpiscale=1} )
      update_canvas()
   end
   return cvs.t.tex
end

function unload ()
   shader = nil
   cvs = nil
end

function update( dt )
   shader:update( dt )
end

function render ()
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   update_canvas()
   cvs:draw( x, y, 0, 1/z )
end

function can_land ()
   print( "can_land" )
   return true, "The wormhole seems to be active."
end

function land ()
   var.push( "wormhole_target", "Gamma Polaris" )
   naev.eventStart("Wormhole")
end
