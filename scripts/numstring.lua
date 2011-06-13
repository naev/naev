-- Converts an integer into a human readable string, delimiting every third digit with a comma.
-- Note: rounds input to the nearest integer. Primary use is for payment descriptions.
function numstring(number)
    number = math.floor(number + 0.5)
    local numberstring = ""
    while number > 1000 do
        local newnumber = number % 1000
        local newsection = "" .. newnumber
        if newnumber == 0 then newnumber = 1 end -- Special hack for a segment of only zeroes.
        while newnumber < 100 do
            newsection = "0" .. newsection
            newnumber = newnumber * 10
        end
        numberstring = "," .. newsection .. numberstring
        number = math.floor(number / 1000)
    end
    numberstring = number % 1000 .. numberstring
    return numberstring
end