--[[

   Custom death animation for Scavenger. Just makes them fade out instead of dying.

--]]
function explode_init( p )
   p:effectAdd( "Fade-Out" )
   mem.timer = 3
end

function explode_update( _p, _dt )
   -- Stub since it's handled in the effect
end
