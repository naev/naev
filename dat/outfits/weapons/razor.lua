local fmt = require "format"

local disable, penetration, isturret, duration, energy
duration = 3
function onload( o )
   local s     = o:specificstats()
   -- The effect applies a fixed ratio of energy vs disable and is normalized
   -- around disable.
   disable     = s.disable*0.5
   energy      = s.disable*1.5
   penetration = s.penetration
   isturret    = s.isturret
end

function descextra( _p )
   return "#y"..fmt.f(_("Ionization deals an extra {disable:.1f} of disable and {energy:.1f} of energy drain over {duration} seconds on the target.").."#0",
      {disable=disable, energy=energy, duration=duration})
end

function onimpact( p, target )
   local ts = target:stats()
   local dmg = disable * (1 - math.min( 1, math.max( 0, ts.absorb - penetration ) ))

   -- Modify by damage
   if p:exists() then
      local mod
      if isturret then
         mod = p:shipstat("tur_damage",true)
      else
         mod = p:shipstat("fwd_damage",true)
      end
      dmg = dmg * mod * p:shipstat("weapon_damage",true)
   end

   target:effectAdd( "Ionization", duration, dmg, p )
end
