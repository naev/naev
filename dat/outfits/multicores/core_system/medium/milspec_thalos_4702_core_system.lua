notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if po:slot().tags.secondary then
      mass=90
      cpu_max=-180
      energy=50
      energy_regen=-3
      shield=-300
      shield_regen=-7
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   end
end
