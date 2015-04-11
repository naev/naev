--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Enumerates the arguments passed to it. Arguments are used as keys and will be assigned numbers in the order they are passed.
-- 
-- Example usage: my_enum = enumerate("first", "second", "third")
-- Example usage: print(my_enum.first)
function enumerate(...)
    local enum = {}
    for i, j in ipairs(arg) do
        enum[j] = i
    end
    return enum
end
