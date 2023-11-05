--local fmt = require "format"

local damage, penetration, isturret
function onload( o )
   local s     = o:specificstats()
   damage      = s.damage
   penetration = s.penetration
   isturret    = s.isturret
end

function descextra( _p )
end

function onimpact( p, target )
   local ts = target:stats()
   local dmg = damage * (1 - math.min( 1, math.max( 0, ts.absorb - penetration ) ))
   local dur = 0

   -- Modify by damage
   if p:exists() then
      local mod
      if isturret then
         mod = p:shipstat("tur_damage",true)
      else
         mod = p:shipstat("fwd_damage",true)
      end
      dmg = dmg * mod
   end

   target:effectAdd( "Plasma Burn", dur, dmg )
end
