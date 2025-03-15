notactive = true

function init( _p, po )
   if not po:slot().tags.secondary then
      mass=8
      cpu_max=16
      energy=150
      energy_regen=7
      shield=150
      shield_regen=4.5
      ew_detect=10
      cooldown_time=-25
   else
      mass=57
      cpu_max=52
      energy=75
      energy_regen=3
      shield=30
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
