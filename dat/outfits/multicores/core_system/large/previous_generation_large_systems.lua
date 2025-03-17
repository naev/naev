notactive = true

function init( _p, po )
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      cpu_max=380
      energy=1760
      energy_regen=35
      shield=500
      shield_regen=9
   else
      cpu_max=0
      energy=0
      energy_regen=0
      shield=0
      shield_regen=0
   end
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
