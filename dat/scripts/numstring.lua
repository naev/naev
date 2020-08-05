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


--[[
-- @brief Properly converts a number of credits to a string, utilizing ngettext.
--
-- This adds "credits" to the output of numstring in a translatable way.
-- Should be used everywhere a number of credits is displayed.
--
-- @usage tk.msg( "", _("You have been paid %s."):format( creditstring(credits) ) )
--
--    @param credits Number of credits.
--    @return A string taking the form of "X credit" or "X credits".
--]]
function creditstring( credits )
   return gettext.ngettext( "%s credit", "%s credits", credits ):format(
         numstring(credits) )
end
