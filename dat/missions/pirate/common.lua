--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Common Pirate Mission framework

   This framework allows to keep consistency and abstracts around commonly used
    Pirate mission functions.

--]]


--[[
-- @brief Gets a random pirate lord portrait name.
--
-- @return A random pirete lord portrait name.
--]]
function pir_getLordRandomPortrait ()
   local portraits = {
      "pirate/pirate1",
      "pirate/pirate2",
      "pirate/pirate3",
      "pirate/pirate4",
   }

   return portraits[ rnd.rnd( 1, #portraits ) ]
end
