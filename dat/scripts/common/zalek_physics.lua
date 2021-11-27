--[[

   Za'lek Particle Physics Common Functions

--]]
local vn = require "vn"
local mt = require 'merge_tables'
local love_shaders = require "love_shaders"
local lg = require "love.graphics"

local zpp = {}

-- Noona Sanderaite
zpp.noona = {
   portrait = "zalek3.png",
   image = "zalek3.png",
   name = _("Noona"),
   color = nil,
   transition = nil, -- Use default
   description = _("You see Noona who looks like she might have a job for you."),
}

function zpp.vn_noona( params )
   return vn.Character.new( zpp.noona.name,
         mt.merge_tables( {
            image=zpp.noona.image,
            color=zpp.noona.colour,
         }, params) )
end

-- Function for adding log entries for miscellaneous one-off missions.
function zpp.log( text )
   shiplog.create( "zlk_physics", _("Particle Physics"), _("Za'lek") )
   shiplog.append( "zlk_physics", text )
end

zpp.rewards = {
   zpp01 = 200e3,
   zpp02 = 300e3,
   zpp03 = 200e3, -- + "Heavy Weapons Combat License" permission
   zpp04 = 400e3,
   --zpp05 = 0, -- No payment, reall small flashback really
   zpp06 = 200e3, -- + "Heavy Combat Vessel License" permission
}

function zpp.shader_focal ()
   local pixelcode = [[
#include "lib/simplex.glsl"

uniform float u_time = 0.0;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = texture_coords * 2.0 - 1.0;
   float m = 1.0 / love_ScreenSize.x;

   vec3 ncoord = vec3(normalize(uv), u_time);

   float dsoft1 = length(uv) - (0.9 + 0.1 * snoise(ncoord));

   float dsoft2 = length(uv) - (0.6 + 0.1 * snoise(ncoord * vec3(2.0) + vec3(100.0)));

   float dhard = length(uv) - (0.15 + 0.1 * snoise(ncoord * vec3(3.0) + vec3(300.0)));

   vec4 colout = color;
   colout *= pow( smoothstep( -dsoft1, dsoft2, 0.0 ), 2.0 );
   colout += smoothstep( -m, 0.0, -dhard );
   colout.a *= smoothstep( -m, 0, -dsoft1 );

   return colout;
}
]]
   local shader = lg.newShader( pixelcode, love_shaders.vertexcode )
   shader._dt = -1000 * rnd.rnd()
   shader.update = function( self, dt )
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   shader.render = function( self, x, y, size )
      local oldshader = lg.getShader()
      lg.setColor( 0, 0.2, 1, 1 )
      lg.setShader( self )
      local s2 = size*0.5
      love_shaders.img:draw( x-s2, y-s2, 0, size, size )
      lg.setShader( oldshader )
   end
   return shader
end

return zpp
