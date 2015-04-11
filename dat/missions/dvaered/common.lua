--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]


--[[
   @brief Increases the reputation limit of the player.
--]]
function dv_modReputation( increment )
   local cur = var.peek("_fcap_dvaered") or 30
   var.push("_fcap_dvaered", math.min(cur+increment, 100) )
end



