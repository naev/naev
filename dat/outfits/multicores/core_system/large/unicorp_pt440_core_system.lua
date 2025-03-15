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

   if not po:slot().tags.secondary then
      mass=420
      cpu_max=440
      energy=1860
      energy_regen=46
      shield=650
      shield_regen=11
      ew_detect=10
      cooldown_time=-25
   else
      mass=580
      cpu_max=1310
      energy=940
      energy_regen=51
      shield=100
      shield_regen=2
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
