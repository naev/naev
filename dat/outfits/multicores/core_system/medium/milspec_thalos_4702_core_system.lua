notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      mass=80
      cpu_max=300
      energy=675
      energy_regen=23
      shield=400
      shield_regen=8
   else
      mass=170
      cpu_max=120
      energy=725
      energy_regen=20
      shield=100
      shield_regen=1
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
