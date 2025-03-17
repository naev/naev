notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 90 )
      po:set( "cpu_max", -160 )
      po:set( "energy", 100 )
      po:set( "energy_regen", -13 )
      po:set( "shield", -320 )
      po:set( "shield_regen", -8 )
   end
end
