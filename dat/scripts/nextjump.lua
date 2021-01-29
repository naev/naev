-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys then
       return nowsys
   end

   path = nowsys:jumpPath( finalsys, hidden )
   if not path then
      return nowsys
   end

   return path[1]:dest()
end
