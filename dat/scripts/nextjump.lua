--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys then
       return nowsys
   end

   path = nowsys:jumpPath( finalsys )
   if not path then
      return nowsys
   end

   return path[1]:dest()
end
