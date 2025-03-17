notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 70 )
      po:set( "cpu_max", -90 )
      po:set( "energy", 50 )
      po:set( "energy_regen", 2 )
      po:set( "shield", -260 )
      po:set( "shield_regen", -5.5 )
      po:set( "ew_detect", -10 )
      po:set( "cooldown_time", 25 )
   end
end
