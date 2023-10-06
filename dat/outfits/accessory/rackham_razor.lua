local pir

function onload( _o )
   pir = pir or require "common.pirate" -- Can't load at the beginning because missing factions
end

function init( p, po )
   if pir.isPirateShip( p ) then
      -- It's multiplicative so we add a bonus such that multiplied by the base bonus we get double the original amount
      po:set( "loot_mod", 120/1.1-100 )
      po:set( "jam_chance", 120/1.1/100 )
   end
end
