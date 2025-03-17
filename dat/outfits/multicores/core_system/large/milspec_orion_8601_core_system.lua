notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 220 )
      po:set( "cpu_max", 1120 )
      po:set( "energy", -1080 )
      po:set( "energy_regen", 8 )
      po:set( "shield", -600 )
      po:set( "shield_regen", -12 )
   end
end
