notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 90 )
      po:set( "cpu_max", -180 )
      po:set( "energy", 50 )
      po:set( "energy_regen", -3 )
      po:set( "shield", -300 )
      po:set( "shield_regen", -7 )
   end
end
