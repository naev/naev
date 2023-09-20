--[[

   Za'lek Particle Physics Common Functions

--]]
local vn = require "vn"
local love_shaders = require "love_shaders"
local love = require "love"
local lg = require "love.graphics"

local zpp = {}

-- Fullname: Noona Sanderaite
zpp.noona = {
   portrait = "noona.png",
   image = "noona.png",
   name = _("Noona"),
   colour = { 1, 0.8, 0.8},
   transition = "hexagon",
   description = _("You see Noona who looks like she might have a job for you."),
}

function zpp.vn_noona( params )
   return vn.Character.new( zpp.noona.name,
         tmerge( {
            image=zpp.noona.image,
            color=zpp.noona.colour,
         }, params) )
end

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
   zpp06 = 500e3, -- + "Heavy Combat Vessel License" permission
}

zpp.fctmod = {
   zpp01 = 2,
   zpp02 = 2,
   zpp03 = 3,
   zpp04 = 2,
   zpp05 = 1,
   zpp06 = 3,
}

function zpp.shader_focal ()
   local pixelcode = [[
#include "lib/simplex.glsl"

uniform float u_time;

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
   love.origin()
   return shader
end

function zpp.shader_nebula ()
   local pixelcode = [[
#include "lib/nebula.glsl"

const float hue         = 300.0;
const float view        = 300.0;
const float sf          = %f;
uniform float u_time;
uniform vec3 u_camera   = vec3(0.0, 0.0, 1.0);
uniform float u_progress;
uniform float u_mode;

vec4 nebula_bg( vec2 screen_coords )
{
   vec2 rel_pos = screen_coords * u_camera.z + u_camera.xy;
   rel_pos /= view;
   return nebula( vec4(0.0, 0.0, 0.0, 1.0), rel_pos, u_time*0.1, hue, 1.0, 0.0, 0.1 );
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = (texture_coords*2.0-1.0) * love_ScreenSize.xy;
   float r = (u_progress*2.0-0.5) * max( love_ScreenSize.x, love_ScreenSize.y ) * M_SQRT2;
   float d = length(uv)-r;

   if (u_mode==0)
      color.a *= smoothstep( -800.0 / sf, 0.0, -abs(d) );
   else if (u_mode==1)
      color.a *= smoothstep( -800.0 / sf, 0.0, -d );
   else if (u_mode==2)
      color.a *= 1.0-smoothstep( -800.0 / sf, 0.0,  -d );

   if (color.a > 0.0) {
      vec4 nebucol = nebula_bg( uv );
      nebucol.a *= color.a;
      return nebucol;
   }
   return color;
}
]]
   local sf = naev.conf().nebu_scale
   pixelcode = string.format( pixelcode, sf )
   local shader = lg.newShader( pixelcode, love_shaders.vertexcode )
   shader.sf = sf
   shader._dt = -1000 * rnd.rnd()
   shader.progress = 0
   shader.speed = 1
   local nw, nh = gfx.dim()
   shader.cw, shader.ch = nw/shader.sf, nh/shader.sf
   shader.canvas = lg.newCanvas( shader.cw, shader.ch )

   shader.update = function( self, dt )
      self._dt = self._dt + dt
      self.progress = self.progress + dt * self.speed
      self:send( "u_time", self._dt )
      self:send( "u_progress", self.progress )
   end
   shader.render = function( self )
      -- TODO we should actually downscale this...
      local cx, cy = camera.get():get()
      local cz = camera.getZoom()
      self:send( "u_camera", {cx, -cy, cz*self.sf} )

      local oldcanvas = lg.getCanvas()
      local oldshader = lg.getShader()
      lg.setColor( 1, 1, 1, 1 )
      lg.setCanvas( self.canvas )
      lg.clear( 0, 0, 0, 0 )
      lg.setShader( self )
      lg.setBlendMode( "alpha", "premultiplied" )
      love_shaders.img:draw( 0, 0, 0, self.cw, self.ch )
      lg.setBlendMode( "alpha" )
      lg.setShader( oldshader )
      lg.setCanvas( oldcanvas )

      -- Render to screen
      self.canvas:draw( 0, 0, 0, self.sf, self.sf )
   end
   shader.reset = function( self, speed )
      speed = speed or 1
      self.progress = 0
      self.speed = speed
      self:update( 0 )
   end
   love.origin()
   return shader
end

return zpp
