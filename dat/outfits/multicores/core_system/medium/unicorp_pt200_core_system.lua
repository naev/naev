notactive = true

function init( _p, po )
   if not po:slot().tags.secondary then
      mass=70
      cpu_max=200
      energy=525
      energy_regen=17
      shield=330
      shield_regen=6.5
      ew_detect=10
      cooldown_time=-25
   else
      mass=140
      cpu_max=110
      energy=575
      energy_regen=19
      shield=70
      shield_regen=1
      ew_detect=0
      cooldown_time=0
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
   po:set( "ew_detect", ew_detect )
   po:set( "cooldown_time", cooldown_time )
end
