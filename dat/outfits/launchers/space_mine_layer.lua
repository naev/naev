local spacemine = require "luaspfx.spacemine"

local ss
function onload( o )
   ss = o:specificstats()
end

function onshoot( p, _po, on )
   if not on then return end

   spacemine( p:pos(), p:vel(), p:faction(), {
      damage      = ss.damage,
      penetration = ss.penetration,
      trackmax    = ss.trackmin,
      trackmin    = ss.trackmax,
      duration    = ss.duration,
      pilot       = p,
   } )

   return true
end
