notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy_regen
   local shield
   local shield_regen

   if po:slot().tags.secondary then
      mass=47
      cpu_max=34
      energy_regen=1
      shield=-150
      shield_regen=-6
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   end
end
