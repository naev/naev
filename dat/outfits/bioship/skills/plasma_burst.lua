local luaspfx = require "luaspfx"
local audio = require "love.audio"

local damage = 100
local penetration = 0.5
local radius = 200
local cooldown = 20

local sfx = audio.newSource( 'snd/sounds/plasma_burst.ogg' )

local function activate( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end

   -- TODO would be great to delay the damage by 0.5 seconds to make it fit better with sound and effects
   local dur = 10
   local pos = p:pos()
   for k,t in ipairs(p:getEnemies( radius )) do
      local ts = t:stats()
      local dmg = damage * (1 - math.min( 1, math.max( 0, ts.absorb - penetration ) ))
      local norm, angle = (t:pos() - pos):polar()
      local mod = 1 - norm / radius
      local mass = math.pow( damage / 15, 2 )
      -- Damage and knockback
      t:damage( damage, 0, penetration, "impact", p )
      t:knockback( mass, vec2.newP( mod*radius, angle ), pos, 1 )
      -- Nasty effects
      t:effectAdd( "Plasma Burn", dur, dmg )
      t:effectAdd( "Paralyzing Plasma", dur )
      t:effectAdd( "Crippling Plasma", dur )
   end

   if mem.isp then
      mem.spfx_start = luaspfx.sfx( true, nil, sfx )
   else
      mem.spfx_start = luaspfx.sfx( p:pos(), p:vel(), sfx )
   end

   luaspfx.explosion( pos, p:vel(), 400, nil, {
      silent    = true,
      speed     = 0.5,
      grain     = 0.3,
      steps     = 8,
      smokiness = 0.4,
      rollspeed = 0.3,
      smokefade = 1.6,
      colorbase = {0.9, 0.1, 0.1, 0.1},
      colorsmoke = {0.6, 0.3, 0.3, 0.25},
   } )

   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   return true
end

function init( p, po )
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( _p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   po:progress( mem.timer / cooldown )
   if mem.timer <= 0 then
      po:state("off")
      mem.timer = nil
   end
end

function ontoggle( p, po, on )
   if on then
      return activate( p, po )
   end
end
