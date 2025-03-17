notactive = true

function init( _p, po )
   local cpu_max
   local energy
   local energy_regen
   local shield
   local shield_regen
   local ew_detect
   local cooldown_time
   local jump_warmup
   local land_delay
   local jump_delay
   local ew_hide

   if not po:slot().tags.secondary then
      cpu_max=10
      energy=140
      energy_regen=7
      shield=130
      shield_regen=4
      ew_detect=15
      cooldown_time=-25
      jump_warmup=-60
      land_delay=-25
      jump_delay=-25
      ew_hide=-20
   else
      cpu_max=0
      energy=0
      energy_regen=0
      shield=0
      shield_regen=0
      ew_detect=0
      cooldown_time=0
      jump_warmup=0
      land_delay=0
      jump_delay=0
      ew_hide=0
   end
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
   po:set( "ew_detect", ew_detect )
   po:set( "cooldown_time", cooldown_time )
   po:set( "jump_warmup", jump_warmup )
   po:set( "land_delay", land_delay )
   po:set( "jump_delay", jump_delay )
   po:set( "ew_hide", ew_hide )
end
