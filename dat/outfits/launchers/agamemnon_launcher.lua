local nuke = require "luaspfx.agamemnon"

-- Second stage nuke
function onimpact( p, _target, pos, vel, _o )
   nuke( p, pos, vel*0.1 )
end
