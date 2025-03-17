notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 200 )
      po:set( "cpu_max", 1440 )
      po:set( "energy", -1240 )
      po:set( "energy_regen", 4 )
      po:set( "shield", -700 )
      po:set( "shield_regen", -14 )
   end
end
