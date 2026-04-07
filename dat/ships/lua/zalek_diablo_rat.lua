update_dt = 1

function init( p )
   update( p )
end

function descextra( _p, _s )
   return "#b".._("Converts all fighter-related stat bonuses into launcher counterparts.").."#0"
end

function update( p )
   p:shippropReset()
   
   local fbay_damage = p:shipstat("fbay_damage", true)
   p:shippropSet( "launch_damage", fbay_damage * 100 - 100 )
   p:shippropSet( "fbay_damage", 100 / fbay_damage - 100 )

   local fbay_health = p:shipstat("fbay_health", true)
   p:shippropSet( "launch_range", fbay_health * 100 - 100 )
   p:shippropSet( "fbay_health", 100 / fbay_health - 100 )

   local fbay_movement = p:shipstat("fbay_movement", true)
   p:shippropSet( "launch_speed", fbay_movement * 100 - 100 )
   p:shippropSet( "launch_accel", fbay_movement * 100 - 100 )
   p:shippropSet( "launch_turn", fbay_movement * 100 - 100)
   p:shippropSet( "fbay_movement", 100 / fbay_movement - 100 )
   
   local fbay_capacity = p:shipstat("fbay_capacity", true)
   p:shippropSet( "ammo_capacity", fbay_capacity * 100 - 100 )
   p:shippropSet( "fbay_capacity", 100 / fbay_capacity - 100 )
   
   local fbay_rate = p:shipstat("fbay_rate", true)
   p:shippropSet( "launch_rate", fbay_rate * 100 - 100 )
   p:shippropSet( "fbay_rate", 100 / fbay_rate - 100 )
   
   local fbay_reload = p:shipstat("fbay_reload", true)
   p:shippropSet( "launch_reload", fbay_reload * 100 - 100 )
   p:shippropSet( "fbay_reload", 100 / fbay_reload - 100 )
end
