--This is literally just the energy harpoon lua. Replace it entirely
function onimpact( _p, target, pos, vel, _o )
   local m = math.min( 1000, target:mass() )
   target:knockback( m, -vel, pos, 0 )
end
