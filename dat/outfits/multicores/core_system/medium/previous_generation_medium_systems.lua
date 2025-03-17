notactive = true

function init( _p, po )
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if not po:slot().tags.secondary then
      cpu_max=180
      energy=580
      energy_regen=23
      shield=310
      shield_regen=7
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
