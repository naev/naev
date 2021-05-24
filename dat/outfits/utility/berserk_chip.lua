local function setup_shader()
   local f = file.new( 'glsl/postprocess.vert' )
   f:open('r')
   return shader.new([[
#version 140
const vec4 colmod = vec4( 1.0, 0.5, 0.5, 1.0 );

uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 color_out;

uniform float u_time = 0;

void main (void)
{
   vec4 mod = mix( vec4(1), colmod, min(3.0*u_time, 1.0) );
   color_out = mod * texture( MainTex, VaryingTexCoord.st );
}
]], "#version 140\n"..f:read() )
end

threshold = 30 -- armour shield damage to turn off at
cooldown = 8 -- cooldown time in seconds
drain_shield = 1.0 / 2.0 -- inverse of number of seconds needed to drain shield
drain_armour = 1.0 / 50.0 -- inverse of number of seconds needed to drain armour (this is accelerated by this amount every second)
ppshader = setup_shader()


function turnon( p, po )
   -- Still on cooldown
   if mem.timer > 0 then
      return false
   end
   -- Must be above armour threshold
   local a, s = p:health()
   if a < threshold then
      return false
   end
   po:state("on")
   po:progress(0) -- No progress so just fill out the bar
   mem.active = true

   -- Apply damaging effect
   local ps = p:stats()
   mem.ainc = ps.armour * drain_armour
   mem.admg = mem.ainc
   po:set( "armour_damage", mem.admg )
   po:set( "shield_usage",  ps.shield * drain_shield ) -- shield gone in 2 secs

   -- Visual effect
   if mem.isp then
      ppshader:send( "u_time", 0 )
      mem.shader = shader.addPPShader( ppshader, "game" )
   end

   return true
end

function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown
   mem.active = false
   if mem.shader then
      shader.rmPPShader( ppshader )
   end
   mem.shader = nil
   po:set( "armour_damage", 0 )
   po:set( "shield_usage",  0 )
   po:set( "launch_damage", -20 )
   po:set( "fwd_damage", -20 )
   po:set( "tur_damage", -20 )
   po:set( "turn_mod", -20 )
   po:set( "thrust_mod", -20 )
   po:set( "speed_mod", -20 )
   return true
end

function init( p, po )
   turnoff()
   mem.timer = 0
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
   if mem.active then
      local a, s = p:health()
      if a < threshold then
         turnoff( p, po )
      else
         mem.admg = mem.admg + mem.ainc * dt
         po:set( "armour_damage", mem.admg )
      end
   else
      mem.timer = mem.timer - dt
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         po:clear() -- clear stat modifications
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
