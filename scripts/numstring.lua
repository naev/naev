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
