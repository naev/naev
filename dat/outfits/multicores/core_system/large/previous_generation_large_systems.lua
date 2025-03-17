notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "cpu_max", -380 )
      po:set( "energy", -1760 )
      po:set( "energy_regen", -35 )
      po:set( "shield", -500 )
      po:set( "shield_regen", -9 )
   end
end
