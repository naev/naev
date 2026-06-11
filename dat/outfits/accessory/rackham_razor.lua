local pir
notactive = true

function onload( _o )
   pir = pir or require "common.pirate" -- Can't load at the beginning because missing factions
end

function init( p, po )
   if pir.isPirateShip( p ) then
      po:set( "loot_mod", 10 )
      po:set( "jam_chance", 10 )
      po:set( "fwd_firerate", 5 )
   else
      po:clear()
   end
end
