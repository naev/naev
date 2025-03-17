notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      mass=500
      cpu_max=680
      energy=2300
      energy_regen=53
      shield=800
      shield_regen=14
   else
      mass=700
      cpu_max=2120
      energy=1060
      energy_regen=57
      shield=100
      shield_regen=0
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
