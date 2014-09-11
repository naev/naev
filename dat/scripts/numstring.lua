--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Converts an integer into a human readable string, delimiting every third digit with a comma.
-- Note: rounds input to the nearest integer. Primary use is for payment descriptions.
function numstring(number)
    number = math.floor(number + 0.5)
    local numberstring = ""
    while number >= 1000 do
        numberstring = string.format( ",%03d%s", number % 1000, numberstring )
        number = math.floor(number / 1000)
    end
    numberstring = number % 1000 .. numberstring
    return numberstring
end
