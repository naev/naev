
-- Note: Lunge is adrenal glands effects +50%. In addition, turn modifier.

local fmt   = require "format"
local osh   = require 'outfits.shaders'
local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local helper = require "outfits.lib.helper"
local constants = require "constants"

local COOLDOWN = 15 -- cooldown time in seconds
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 colour     = texture( tex, texcoord );
   float opacity   = clamp( progress, 0.0, 1.0 );
   colour.rgb      = blendSoftLight( colour.rgb, colmod, opacity );
   return colour;
}
]])

local sfx_start = audio.newSoundData( 'snd/sounds/growl1.ogg' )
local sfx_bite = audio.newSoundData( 'snd/sounds/crash1.ogg' )

local function turnoff_afterburner()
   local pp = player.pilot()
   for _i,n in ipairs(pp:actives()) do
      if n.outfit:tags().movement and n.state=="on" then
         if not pp:outfitToggle( n.slot ) then -- Failed to disable
            return true
         end
      end
   end
end

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
            helper.msgnospam("#r".._("You need a target to bite!"))
         end
         return false
      end
      mem.isasteroid = true
   end

   -- Must be roughly in front
   local tp = t:pos()
   local _m, a = (p:pos()-tp):polar()
   if math.abs(math.fmod(p:dir()-a, math.pi*2)) > 30/math.pi then
      return false
   end

   -- Try to turn off afterburner
   if turnoff_afterburner() then
      return false
   end

   -- Set bonuses
   po:state("on")
   po:clear()
   po:set( "accel_mod", constants.BITE_ACCEL_MOD )
   po:set( "speed_mod", constants.BITE_SPEED_MOD )
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
   po:clear()

   po:state("cooldown")
   po:progress(1)
   mem.timer = COOLDOWN * p:shipstat("cooldown_mod",true)
   mem.active = false
   p:control(false)
   oshader:off()
   return true
end


--local o_base = outfit.get("The Bite")
local o_lust = outfit.get("The Bite - Blood Lust")
local o_can = outfit.get("The Bite - Cannibal")
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

   if (o==o_can) or mem.lust then
      if mem.improved then
         mem.regen=0.25
      else
         mem.regen=0.1
      end
   else
      mem.regen=0.0
   end

   if mem.lust then
      mem.duration = 5
   else
      mem.duration = 3
   end
end

function descextra( p, o )
   if p then
      local mass = p:mass()
      local values={
         dmg = 10 * math.sqrt(mass),
         accel_mod = constants.BITE_ACCEL_MOD,
         speed_mod = constants.BITE_SPEED_MOD,
         duration = 3,
         mass = fmt.tonnes_short(mass),
         heal = 10,
         cooldown = COOLDOWN,
         bloodlust_dam = 25,
         bloodlust_duration = 10,
      }

      local improved = (o==o_improved)
      local lust = improved or (o==o_lust)
      local can = (o==o_can)

      if lust then
         values.duration = 5
      end
      if improved then
         values.dmg = values.dmg * 1.5
         values.heal = 25
      end
      values.dmg= fmt.number(values.dmg)

      local des=fmt.f(_(
[[#gAcceleration: +{accel_mod}%#0
#gSpeed: +{speed_mod}%#0
#gDuration: {duration} sec or until the target ship is bitten#0
#oCooldown: {cooldown} sec.#0
The ship will lunge at the target enemy and take a huge bite out of it, dealing #g{dmg} damage#0 with its current mass ({mass}).
]]),values)

      if improved then
         return des..fmt.f(_("On successful bite, weapon damage is increased by {bloodlust_dam}% for #g{bloodlust_duration} seconds#0 #p[Blood Lust]#0 and {heal}% of bitten armour is restored to the ship #p[Strong Jaws]#0."),values)
      elseif lust then
         return des..fmt.f(_("On successful bite, weapon damage is increased by #g+{bloodlust_dam}%#0 for #g{bloodlust_duration} seconds#0 #p[Blood Lust]#0 and #g{heal}%#0 of bitten armour is restored to the ship #p[Cannibal II]#0."),values)
      elseif can then
         return des..fmt.f(_("On successful bite, #g{heal}%#0 of bitten armour is restored to the ship #p[Cannibal II]#0"),values)
      else
         return des
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
            if mem.regen>0 then
               p:addHealth( mem.regen * dmg )
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
            if mem.regen>0 then
               ta = t:health(true)
            end
            if mem.improved then
               dmg = dmg*1.5
            end
            if mem.lust then
               p:effectAdd( "Blood Lust" )
            end
            t:damage( dmg, 0, 100, "kinetic", p )
            t:knockback( p, 0.5 )
            -- Do the healing
            if mem.regen>0 then
               p:addHealth( mem.regen * (ta - t:health(true)) )
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
      po:progress( mem.timer / COOLDOWN )
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
   else
      mem.lastmsg = nil -- clear helper.msgnospam timer
      -- Can't turn off the bite.
      --return turnoff( p, po )
   end
end
