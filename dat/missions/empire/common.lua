--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Common Empire Mission framework

   This framework allows to keep consistency and abstracts around commonly used
    empire mission functions.

--]]


--[[
-- @brief Gets a random official portrait name.
--
-- @return A random official portrait name.
--]]
function emp_getOfficialRandomPortrait ()
   local portraits = {
      "empire/empire1",
      "empire/empire2",
      "empire/empire3",
      "empire/empire4"
   }

   return portraits[ rnd.rnd( 1, #portraits ) ]
end


--[[
   @brief Increases the reputation limit of the player.
--]]
function emp_modReputation( increment )
   local cur = var.peek("_fcap_empire") or 30
   var.push("_fcap_empire", math.min(cur+increment, 100) )
end


