require "outfits.lib.matrix_sell"

local REF
local RANGE = 3000 -- TODO compute
local OFFSET = math.pi/4
local onload_old = onload
function onload( o )
   REF = o
   onload_old( o )
end

function onshoot( p, po )
   local t = p:target()
   local dir = p:dir()
   local pos = po:mount( p )
   local vel = p:vel()

   local elst = p:getEnemies( RANGE )
   table.insert( elst, t )
   local n = #elst

   po:munition( p, REF, t, dir, pos, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir+OFFSET,   pos, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir-OFFSET,   pos, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir+2*OFFSET, pos, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir-2*OFFSET, pos, vel, true )

   return true
end

function onimpact( _p, target )
   target:effectAdd( "Shield Jammed" )
end
