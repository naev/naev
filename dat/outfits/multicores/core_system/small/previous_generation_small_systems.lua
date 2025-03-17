notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "cpu_max", -10 )
      po:set( "energy", -100 )
      po:set( "energy_regen", -5 )
      po:set( "shield", -110 )
      po:set( "shield_regen", -4 )
   end
end
