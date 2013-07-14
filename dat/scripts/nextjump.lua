-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys then
       return nowsys
   end

   local neighs    = nowsys:adjacentSystems( hidden )
   local nearest   = 1e9 -- Large value
   local mynextsys = finalsys
   for _, j in pairs(neighs) do
      local jdist = j:jumpDist( finalsys, hidden )
      if jdist < nearest then
         nearest     = jdist
         mynextsys   = j
      end
   end
   return mynextsys
end
