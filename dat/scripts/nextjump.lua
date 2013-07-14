-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys then
       return nowsys
   end

   print( string.format( "Getting next jump from %s en route to %s", nowsys:name(), finalsys:name() ) )
   local neighs    = nowsys:adjacentSystems( hidden )
   local nearest   = 1e9 -- Large value
   local mynextsys = finalsys
   for _, j in pairs(neighs) do
      local jdist = j:jumpDist( finalsys, hidden )
      print( string.format( "   %s - %d", j:name(), jdist ) )
      if jdist < nearest then
         nearest     = jdist
         mynextsys   = j
      end
   end
   print( string.format( "Chose %s - %d", mynextsys:name(), nearest ) )
   return mynextsys
end
