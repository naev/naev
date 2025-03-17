notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 46 )
      po:set( "cpu_max", 42 )
      po:set( "energy", 50 )
      po:set( "energy_regen", -1 )
      po:set( "shield", -135 )
      po:set( "shield_regen", -5 )
   end
end
