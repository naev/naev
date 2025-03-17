notactive = true

function init( _p, po )
   if po:slot().tags.secondary then
      po:set( "mass", 160 )
      po:set( "cpu_max", 870 )
      po:set( "energy", -920 )
      po:set( "energy_regen", 5 )
      po:set( "shield", -550 )
      po:set( "shield_regen", -9 )
      po:set( "ew_detect", -10 )
      po:set( "cooldown_time", 25 )
   end
end
