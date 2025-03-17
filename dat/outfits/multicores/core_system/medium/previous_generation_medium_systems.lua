notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "cpu_max", -180 )
      po:set( "energy", -580 )
      po:set( "energy_regen", -23 )
      po:set( "shield", -310 )
      po:set( "shield_regen", -7 )
   end
end
