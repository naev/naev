local osh   = require 'outfits.shaders'
local audio = require 'love.audio'

local cooldown = 15 -- cooldown time in seconds
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = clamp( progress, 0.0, 1.0 );
   color.rgb      = blendSoftLight( color.rgb, colmod, opacity );
   return color;
}
]])

local sfx_start = audio.newSource( 'snd/sounds/growl1.ogg' )
local sfx_bite = audio.newSource( 'snd/sounds/crash1.ogg' )

local function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   -- Needs a target
   local t = p:target()
   if t==nil then
      return false
   end
   -- Must be roughly infront
   local tp = t:pos()
   local _m, a = (p:pos()-tp):polar()
   if math.abs(math.fmod(p:dir()-a, math.pi*2)) > 30/math.pi then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.timer = mem.duration
   mem.active = true

   p:control(true)
   p:pushtask( "lunge", t )

   -- Visual effect
   if mem.isp then
      sfx_start:setPitch( player.dt_mod() )
      sfx_start:play()
      oshader:on()
   end

   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown
   mem.active = false
   p:control(false)
   oshader:off()
   return true
end


--local o_base = outfit.get("The Bite")
local o_lust = outfit.get("The Bite - Blood Lust")
local o_improved = outfit.get("The Bite - Improved")
function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
   oshader:force_off()

   local o = po:outfit()
   mem.improved = (o==o_improved)
   mem.lust = mem.improved or (o==o_lust)

   if mem.lust then
      mem.duration = 5
   else
      mem.duration = 3
   end
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   if mem.active then
      oshader:update_on(dt)
      if mem.timer <= 0 then
         return turnoff( p, po )
      else
         local t = p:target()
         if t==nil then
            return turnoff( p, po )
         end
         local c = p:collisionTest( t )
         if not c then
            po:progress( mem.timer / mem.duration )
         else
            -- Hit the enemy!
            local dmg = 10*math.sqrt(p:mass())
            local ta = t:health(true)
            if mem.improved then
               dmg = dmg*1.5
            end
            if mem.lust then
               p:effectAdd( "Blood Lust" )
            end
            t:damage( dmg, 0, 100, "impact", p )
            t:knockback( p, 0.5 )
            -- Do the healing
            if mem.improved then
               local heal = 0.25 * (ta - t:health(true))
               local pa = p:health()
               local ps = p:stats()
               p:setHealth( math.min(100, pa+100*heal/ps.armour) )
            end
            -- Player effects
            if mem.isp then
               sfx_start:stop()
               sfx_bite:setPitch( player.dt_mod() )
               sfx_bite:play()
               camera.shake( 0.8 )
            end
            return turnoff( p, po )
         end
      end
   else
      oshader:update_cooldown(dt)
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
         oshader:force_off()
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   --else
   --   return turnoff( p, po )
   end
end
