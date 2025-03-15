notactive = true

function init( _p, po )
   if not po:slot().tags.secondary then
      mass=90
      cpu_max=260
      energy=750
      energy_regen=33
      shield=450
      shield_regen=10
   else
      mass=180
      cpu_max=100
      energy=850
      energy_regen=20
      shield=130
      shield_regen=2
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
