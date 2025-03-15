notactive = true

function init( _p, po )
   local mass
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen
   local ew_detect
   local cooldown_time

   if po:slot().tags.secondary then
      mass=49
      cpu_max=36
      energy=-75
      energy_regen=-4
      shield=-120
      shield_regen=-3.5
      ew_detect=-10
      cooldown_time=25
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
      po:set( "ew_detect", ew_detect )
      po:set( "cooldown_time", cooldown_time )
   end
end
