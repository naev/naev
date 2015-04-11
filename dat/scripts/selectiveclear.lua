--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Removes all pilots belonging to a faction from the system.
-- Takes the name of the faction, as a string.
-- 
-- Example usage: pilot.clearSelect("Empire")
function pilot.clearSelect(facname)
   local ps = pilot.get({faction.get(facname)})
   for _, j in ipairs(ps) do
      if j:exists() then
         j:rm()
      end
   end
end
