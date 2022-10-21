-- Global constant variables for the outfit
local range = 2000
local bonus = 10

-- Only care about fighting classes
local function pilotToLevel( p )
   return p:ship():size()
end

-- Init function run on creation
function init( p, po )
   mem.active = false
   po:state( "off" )
   mem.pl = pilotToLevel( p )
   mem.nearby = 0
end

function update( p, po, _dt )
   local h = p:getEnemies(range) -- Only consider visible ships
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
            po:set( "fwd_damage", n*bonus )
            po:set( "tur_damage", n*bonus )
            po:set( "launch_damage", n*bonus )
         end
      else
         po:state( "off" )
      end
      mem.nearby = n
   end
end
