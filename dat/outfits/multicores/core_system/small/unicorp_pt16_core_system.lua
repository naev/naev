notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 49 )
      po:set( "cpu_max", 36 )
      po:set( "energy", -75 )
      po:set( "energy_regen", -4 )
      po:set( "shield", -120 )
      po:set( "shield_regen", -3.5 )
      po:set( "ew_detect", -10 )
      po:set( "cooldown_time", 25 )
   end
end
