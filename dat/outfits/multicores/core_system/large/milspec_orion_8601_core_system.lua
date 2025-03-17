notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      mass=540
      cpu_max=540
      energy=2460
      energy_regen=66
      shield=850
      shield_regen=15
   else
      mass=760
      cpu_max=1660
      energy=1380
      energy_regen=74
      shield=250
      shield_regen=3
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
