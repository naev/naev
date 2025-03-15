notactive = true

function init( _p, po )
   if not po:slot().tags.secondary then
      mass=14
      cpu_max=18
      energy_regen=10
      shield=200
      shield_regen=7
   else
      mass=61
      cpu_max=52
      energy_regen=11
      shield=50
      shield_regen=1
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
