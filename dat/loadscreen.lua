local lg = require "love.graphics"
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local prng = require("prng").new()

local progressbar_frag = [[
#include "lib/nebula.glsl"
#include "lib/sdf.glsl"

uniform vec2 dimensions;
uniform float progress;
uniform float u_r;
uniform float time;

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords ) {
   vec4 colour_out;
   vec2 rel_pos = gl_FragCoord.xy * 0.05 + u_r*100.0;

   const float margin = 0.05;
   float relprog = smoothstep( -margin, margin, texture_coords.x-progress);

   const float hue   = 0.65;
   float value       = 0.4*(1.0-relprog);
   float brightness  = 0.1*relprog;

   colour_out = nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, time, hue, value, 0.0, brightness );
   colour_out *= 1.0 - 0.8 * relprog;

   const float b = 8.0;
   float dist = sdBox( (texture_coords.xy*2.0-1.0)*dimensions, vec2(dimensions.x-2.0*b,dimensions.y-2.0*b) );
   dist = 1.0 - dist / b * 0.5;
   colour_out.a *= dist;
   return colour_out;
}
]]

local function load_shader ()
   local starfield_frag = lf.read('bkg/shaders/starfield.frag')

   prng:setSeed( 43 )
   local motionblur = 0
   local theta = prng:random() * math.pi/10.0
   local phi = prng:random() * math.pi/10.0
   local psi = prng:random() * math.pi/10.0
   local rx, ry = vec2.newP( 3+1*prng:random(), 7+1*prng:random() ):get()
   local rz = 5+1*prng:random()
   local sz = 1+1*prng:random()
   local shader = lg.newShader( string.format(starfield_frag, motionblur, rx, ry, rz, theta, phi, psi), love_shaders.vertexcode )

   local nw, nh = naev.gfx.dim()

   local texw = nw
   local texh = nh
   local texs = 4096 / math.max( texw, texh )
   if texs < 1 then
      texw = texw / texs
      texh = texh / texs
   end
   local cvs = lg.newCanvas( texw, texh, {dpiscale=1} )
   shader:send( "u_camera", 0, 0, sz, 0.0008 )

   local oldcanvas = lg.getCanvas()
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setShader( shader )
   lg.setColour( {1,1,1,1} )
   love_shaders.img:draw( 0, 0, 0, texw, texh )
   lg.setShader()
   lg.setCanvas( oldcanvas )

   return cvs
end

local function load_gfx ()
   local basepath = "gfx/loading/"
   local files = {}
   for k,f in ipairs( lf.getDirectoryItems( basepath ) ) do
      if string.match( f, ".webp$" ) then
         table.insert( files, f )
      end
   end

   local name = files[ rnd.rnd(1,#files) ]
   local path = basepath..name

   local caption = lf.read( path..".txt" )
   local image = lg.newImage( path )

   -- Remove trailing whitespace. TODO why do we have to do this? :/
   caption = string.gsub( caption, '^%s*(.-)%s*$', '%1' )

   return image, _(caption)
end

local fh = 24
local font = lg.newFont( fh )
local bg = load_shader()
local shipgfx, shipcaption = load_gfx()
local captionw = font:getWidth( shipcaption )
local shipw, shiph = shipgfx:getDimensions()
local progressbar = lg.newShader( progressbar_frag, love_shaders.vertexcode )
progressbar:send( "u_r", rnd.rnd() )
local sb = naev.conf().bg_brightness
local load_msg = ""

function update( done, msg )
   load_msg = msg
   progressbar:send( "progress", done )
end

local lw, lh = 0, 0
local bx, by, w, h, x, y
function render ()
   local nw, nh = naev.gfx.dim()
   if nw~=lw or nh~=lh then
      lw, lh = nw, nh

      -- Dimensions
      bx = (nw-shipw)/2
      by = (nh-shiph)/2-50
      w = nw * 0.5
      h = nh * 0.025
      x = (nw-w)/2
      y = by + shiph + h + 10

      progressbar:send( "dimensions", w, h )
   end

   -- Draw starfield background
   lg.setColour( sb, sb, sb, 1 )
   bg:draw( 0, 0, 0, 1, 1 )

   -- Draw ship
   lg.setColour( 1, 1, 1, 1 )
   shipgfx:draw( bx, by )
   lg.print( shipcaption, font, bx+shipw-captionw, by+shiph-20 )

   -- Draw loading bar
   lg.setShader( progressbar )
   progressbar:send( "time", naev.ticks()*0.3 )
   love_shaders.img:draw( x, y+h, 0, w, h )
   lg.setShader()
   lg.print( load_msg, x+10, y )
end
