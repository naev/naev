notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen

   if po:slot().tags.secondary then
      mass=220
      cpu_max=1120
      energy=-1080
      energy_regen=8
      shield=-600
      shield_regen=-12
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   end
end
