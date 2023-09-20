local fmt   = require "format"
local osh   = require 'outfits.shaders'
local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local cooldown = 15 -- cooldown time in seconds
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress;
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
   mem.isasteroid = false
   if t==nil then
      t = p:targetAsteroid()
      if t==nil then
         if mem.isp then
            player.msg("#r".._("You need a target to bite!"))
         end
         return false
      end
      mem.isasteroid = true
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
   mem.target = t

   p:control(true)
   p:pushtask( "lunge", t )

   -- Visual effect
   if mem.isp then
      oshader:on()
      mem.spfx_start = luaspfx.sfx( true, nil, sfx_start )
   else
      mem.spfx_start = luaspfx.sfx( p:pos(), p:vel(), sfx_start )
   end

   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   mem.active = false
   p:control(false)
   oshader:off()
   return true
end


--local o_base = outfit.get("The Bite")
local o_lust = outfit.get("The Bite - Blood Lust")
local o_improved = outfit.get("The Bite - Improved")
function init( p, po )
   turnoff( p, po )
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

function descextra( p, o )
   if p then
      local mass = p:mass()
      local dmg = 10*math.sqrt(mass)
      local dur = 3
      local improved = (o==o_improved)
      local lust = improved or (o==o_lust)
      if lust then
         dur = 5
      end
      if improved then
         dmg = dmg * 1.5
      end
      dmg = "#o"..fmt.number(dmg).."#0"

      if improved then
         return fmt.f(_("Makes the ship lunge for {duration} seconds at the target to take a bite out of it for {damage} damage ({mass}) [Strong Jaws]. On succesful bite, weapon damage is increased by 25% for 10 seconds [Blood Lust], and 25% of bitten armour is restored to the ship [Strong Jaws]."),
            {damage=dmg, mass=fmt.tonnes_short(mass), duration=dur } )
      elseif lust then
         return fmt.f(_("Makes the ship lunge for {duration} seconds at the target to take a bite out of it for {damage} damage ({mass}). On succesful bite, weapon damage is increased by 25% for 10 seconds [Blood Lust]."),
            {damage=dmg, mass=fmt.tonnes_short(mass), duration=dur } )
      else
         return fmt.f(_("Makes the ship lunge at the target for {duration} seconds to take a bite out of it for {damage} damage ({mass})."),
            {damage=dmg, mass=fmt.tonnes_short(mass), duration=dur } )
      end
   end
   return _("Makes the ship lunge at the target to take a bite out of it. Damage is based on ship's mass.")
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   if mem.active then
      oshader:update_on(dt)
      if mem.timer <= 0 then
         return turnoff( p, po )
      else
         local t = mem.target
         if t==nil or not t:exists() then
            return turnoff( p, po )
         end
         local c = p:collisionTest( t )
         if not c then
            po:progress( mem.timer / mem.duration )
         elseif mem.isasteroid then
            -- Hit the enemy!
            local dmg = 10*math.sqrt(p:mass())
            local ta = t:armour()
            if mem.improved then
               dmg = dmg*1.5
            end
            t:setArmour( ta-dmg )
            -- TODO better calculation of asteroid mass
            p:knockback( 1000, t:vel(), t:pos(), 0.5 )
            -- Do the healing
            if mem.improved then
               local heal = 0.25 * dmg
               p:addHealth( heal )
            end
            -- Player effects
            mem.spfx_start:rm()
            if mem.isp then
               luaspfx.sfx( true, nil, sfx_bite )
               camera.shake( 0.8 )
            else
               luaspfx.sfx( p:pos(), p:vel(), sfx_bite )
            end
            return turnoff( p, po )
         else
            -- Hit the enemy!
            local dmg = 10*math.sqrt(p:mass())
            local ta
            if mem.improved then
               dmg = dmg*1.5
               ta = t:health(true)
            end
            if mem.lust then
               p:effectAdd( "Blood Lust" )
            end
            t:damage( dmg, 0, 100, "impact", p )
            t:knockback( p, 0.5 )
            -- Do the healing
            if mem.improved then
               local heal = 0.25 * (ta - t:health(true))
               p:addHealth( heal )
            end
            -- Player effects
            mem.spfx_start:rm()
            if mem.isp then
               luaspfx.sfx( true, nil, sfx_bite )
               camera.shake( 0.8 )
            else
               luaspfx.sfx( p:pos(), p:vel(), sfx_bite )
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
