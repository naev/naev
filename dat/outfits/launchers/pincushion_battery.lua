require "outfits.lib.matrix_sell"

local REF
local RANGE = 3000 -- TODO compute
local OFFSET = math.pi/3
local onload_old = onload
function onload( o )
   REF = o
   onload_old()
end

function onshoot( p, po, on )
   if not on then return end

   local t = p:target()
   local dir = p:dir()
   local pos = p:pos()
   local vel = p:vel()

   local elst = p:getEnemies( RANGE )
   table.insert( elst, t )
   local n = #elst

   po:munition( p, REF, t, dir, pos, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir, pos+OFFSET,   vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir, pos-OFFSET,   vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir, pos+2*OFFSET, vel, true )
   po:munition( p, REF, elst[rnd.rnd(1,n)], dir, pos-2*OFFSET, vel, true )

   return true
end
