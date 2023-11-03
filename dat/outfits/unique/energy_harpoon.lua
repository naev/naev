function onimpact( _p, target, pos, vel, _o )
   -- Inelastic collision
   target:knockback( 200, vel, pos, 0 )
end
