local spacemine = require "luaspfx.spacemine"

local ss
function onload( o )
   ss = o:specificstats()
end

function onshoot( p, po )
   spacemine( po:mount(p), p:vel(), p:faction(), {
      damage      = ss.damage,
      penetration = ss.penetration,
      trackmax    = ss.trackmin,
      trackmin    = ss.trackmax,
      duration    = ss.duration,
      pilot       = p,
   } )

   return true
end
