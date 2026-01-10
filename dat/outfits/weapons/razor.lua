local fmt = require "format"

local disable, duration, energy
duration = 3
function onload( o )
   local s     = o:specificstats()
   -- The effect applies a fixed ratio of energy vs disable and is normalized
   -- around disable.
   disable     = s.disable*0.5
   energy      = s.disable*1.5
end

function descextra( _p )
   return "#y"..fmt.f(_("Ionization deals an extra {disable:.1f} of disable and {energy:.1f} of energy drain over {duration} seconds on the target.").."#0",
      {disable=disable, energy=energy, duration=duration})
end

function onimpact( p, target, _pos, _vel, _o, armour, shield )
   local dmg = armour + shield
   target:effectAdd( "Ionization", duration, dmg, p )
end
