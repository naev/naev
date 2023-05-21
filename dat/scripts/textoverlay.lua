local lg = require 'love.graphics'
local love_math = require 'love.math'
local love_shaders = require 'love_shaders'
local transitions = require 'vn.transitions'

local textoverlay = {}

local nw, nh
local textcanvas, textshader, texttimer, text_fadein, text_fadeout, text_length
local hookfg, hookupdate

function textoverlay.init( titletext, subtitletext, opts )
   opts = opts or {}
   text_fadein = opts.fadein or 1.5
   text_fadeout = opts.fadeout or 1.5
   text_length = opts.length or 10.0

   nw, nh = gfx.dim()

   local title, subtitle
   --local fontname = _("fonts/CormorantUnicase-Medium.ttf")
   -- Title
   title = { text=titletext, h=48 }
   title.font = lg.newFont( title.h )
   --title.font = lg.newFont( fontname, title.h )
   title.font:setOutline(3)
   title.w = title.font:getWidth( title.text )
   -- Subtitle
   if subtitletext then
      subtitle = { text=subtitletext, h=32 }
      subtitle.font = lg.newFont( subtitle.h )
      --subtitle.font = lg.newFont( fontname, subtitle.h )
      subtitle.font:setOutline(2)
      subtitle.w = subtitle.font:getWidth( subtitle.text )
   end

   local oldcanvas = lg.getCanvas()
   local emptycanvas = lg.newCanvas()
   lg.setCanvas( emptycanvas )
   lg.clear( 0, 0, 0, 0 )
   lg.setCanvas( oldcanvas )

   -- TODO probably rewrite the shader as this is being computed with the full
   -- screen resolution, breaks with all transitions that use love_ScreenSize...
   textshader  = transitions.get( "perlin" )
   textshader:send( "texprev", emptycanvas )
   textshader._emptycanvas = emptycanvas
   texttimer   = 0

   -- Render to canvas
   local pixelcode = string.format([[
precision highp float;

#include "lib/simplex.glsl"

const float u_r = %f;
const float u_sharp = %f;

float vignette( vec2 uv )
{
   uv *= 1.0 - uv.yx;
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.5); // change pow for modifying the extend of the  vignette
   return vig;
}

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolor = color * texture( tex, uv );

   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * u_sharp * 0.003 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolor.a *= 0.4*n+0.8;
   texcolor.a *= vignette( uv );
   texcolor.rgb *= 0.0;

   return texcolor;
}
]], love_math.random(), 3 )
   local shader = lg.newShader( pixelcode, love_shaders.vertexcode )
   local w, h
   if subtitle then
      w = math.max( title.w, subtitle.w )*1.5
      local _mw, st = subtitle.font:getWrap( subtitle.text, w )
      h = (title.h * 1.5 + subtitle.h*#st)*2
   else
      w = title.w*1.5
      h = title.h*2
   end
   textcanvas = love_shaders.shader2canvas( shader, w, h )

   lg.setCanvas( textcanvas )
   title.x = (w-title.w)/2
   title.y = h*0.2
   lg.print( title.text, title.font, title.x, title.y )
   if subtitle then
      subtitle.x = (w-subtitle.w)/2
      subtitle.y = title.y + title.h*1.5
      lg.print( subtitle.text, subtitle.font, subtitle.x, subtitle.y )
   end
   lg.setCanvas()

   if not hookfg then
      hookfg = hook.renderfg( "_textoverlay_fg" )
      hookupdate = hook.update( "_textoverlay_update" )
   end
end

function _textoverlay_fg ()
   local progress
   if texttimer < text_fadein then
      progress = texttimer / text_fadein
   elseif texttimer > text_length-text_fadeout then
      progress = (text_length-texttimer) / text_fadeout
   end

   if progress then
      lg.setShader( textshader )
      textshader:send( "progress", progress )
   end

   lg.setColor( 1, 1, 1, 1 )

   local x = (nw-textcanvas.w)*0.5
   local y = (nh-textcanvas.h)*0.3
   x = math.floor(x)
   y = math.floor(y)
   lg.draw( textcanvas, x, y )
   if progress then
      lg.setShader()
   end
end

function _textoverlay_update( dt, _real_dt )
   -- We want to show it regardless of the time compression and such
   -- TODO Why is real_dt not equal to dt / player.dt_mod()? :/
   texttimer = texttimer + dt / player.dt_mod()

   if texttimer > text_length then
      hook.rm( hookfg )
      hook.rm( hookupdate )
      hookfg = nil
      hookupdate = nil
      textshader = nil
      textcanvas = nil
   end
end

return textoverlay
