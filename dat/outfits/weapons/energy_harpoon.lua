function onimpact( _p, target, pos, vel, _o )
   -- Inelastic collision
   local m = math.min( 1000, target:mass() )
   target:knockback( m, -vel, pos, 0 )
end
