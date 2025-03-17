notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      mass=12
      cpu_max=24
      energy=150
      energy_regen=8
      shield=180
      shield_regen=6
   else
      mass=58
      cpu_max=66
      energy=200
      energy_regen=7
      shield=45
      shield_regen=1
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
