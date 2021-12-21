local damage
function onload( o )
   local s = o:specificstats()
   damage = s.damage
end

function init( _p, _po )
   -- TODO handle intrinsic bonus outfits here
end

function onhit( _p, _po, target )
   target:effectAdd( "Plasma Burn", damage )
end
