-- Global constant variables for the outfit
range = 2000
bonus = 10

-- Only care about fighting classes
function pilotToLevel( p )
   local c = p:ship():class()
   if c=="Fighter" or c=="Bomber" then
      return 1
   elseif c=="Corvette" then
      return 2
   elseif c=="Destroyer" then
      return 3
   elseif c=="Cruiser" or c=="Carrier" then
      return 4
   end
   return 0
end

-- Init function run on creation
function init( p, po )
   mem.active = false
   po:state( "off" )
   mem.pl = pilotToLevel( p )
   mem.nearby = 0
end

function update( p, po, dt )
   local h = p:getHostiles( range )
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
