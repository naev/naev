notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 47 )
      po:set( "cpu_max", 34 )
      po:set( "energy_regen", 1 )
      po:set( "shield", -150 )
      po:set( "shield_regen", -6 )
   end
end
