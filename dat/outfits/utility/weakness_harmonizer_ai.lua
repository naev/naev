local fmt = require "format"

notactive = true

-- Global constant variables for the outfit
local RANGE = 3500
local BONUS = 10

-- Only care about fighting classes
local function pilotToLevel( p )
   return p:ship():size()
end

function descextra( _p, _o, _po )
   return fmt.f(_([[{bonus}% additional damage for each hostile ship of same class or higher within {range} distance. Range is affected by detection bonus.]]),
      {bonus=BONUS, range=RANGE})
end

-- Init function run on creation
function init( p, po )
   mem.active = false
   po:state( "off" )
   po:clear()
   mem.pl = pilotToLevel( p )
   mem.nearby = 0
end

function update( p, po, _dt )
   local mod = p:shipstat("ew_detect",true)
   local h = p:getEnemies(RANGE*mod) -- Only consider visible ships
   local n = 0
   for k,v in ipairs(h) do
      local l = pilotToLevel( v )
      if l >= mem.pl then
         n = n+1
      end
   end
   -- Something changed
   if n ~= mem.nearby then
      if n > 0 then
         po:state( "on" )
         -- Add extra bonus
         if n > 1 then
            n = n-1
            po:set( "weapon_damage", n*BONUS )
         end
      else
         po:state( "off" )
         po:clear()
      end
      mem.nearby = n
   end
end
