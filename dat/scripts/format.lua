local format = {}

-- Converts an integer into a human readable string, delimiting every third digit with a comma.
-- Note: rounds input to the nearest integer. Primary use is for payment descriptions.
function format.number(number)
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
-- @brief Properly converts a number of credits to a string.
--
-- Should be used everywhere a number of credits is displayed.
--
-- @usage tk.msg("", _("You have been paid %s."):format(fmt.credits(credits)))
--
--    @param credits Number of credits.
--    @return A string taking the form of "X ¤".
--]]
function format.credits( credits )
   return n_("%s ¤", "%s ¤", credits):format( format.number(credits) )
end


--[[
-- @brief Properly converts a number of tonnes to a string, utilizing ngettext.
--
-- This adds "tonnes" to the output of fmt.number in a translatable way.
-- Should be used everywhere a number of tonnes is displayed.
--
-- @usage tk.msg("", _("You are carrying %s."):format(fmt.tonnes(tonnes)))
--
--    @param tonnes Number of tonnes.
--    @return A string taking the form of "X tonne" or "X tonnes".
--]]
function format.tonnes( tonnes )
   return n_("%s tonne", "%s tonnes", tonnes):format( format.number(tonnes) )
end


--[[
-- @brief Like fmt.tonnes, but for abbreviations.
--
--    @param tonnes Number of tonnes.
--    @return A short string like "22 t" describing the given mass.
--]]
function format.tonnes_short( tonnes )
   -- Translator note: this form represents an abbreviation of "_ tonnes".
   return n_( "%d t", "%d t", tonnes ):format( tonnes )
end


--[[
-- @brief Properly converts a number of jumps to a string, utilizing ngettext.
--
-- This adds "jumps" to the output of fmt.number in a translatable way.
-- Should be used everywhere a number of jumps is displayed.
--
-- @usage tk.msg("", _("The system is %s away."):format(fmt.jumps(jumps)))
--
--    @param jumps Number of jumps.
--    @return A string taking the form of "X jump" or "X jumps".
--]]
function format.jumps( jumps )
   return n_("%s jump", "%s jumps", jumps):format(format.number(jumps))
end


--[[
-- @brief Properly converts a number of times (occurrences) to a string,
-- utilizing ngettext.
--
-- This adds "times" to the output of fmt.number in a translatable way.
-- Should be used everywhere a number of occurrences is displayed.
--
-- @usage tk.msg("", _("Brush your teeth % per day."):format(fmt.times(times)))
--
--    @param times Number of times.
--    @return A string taking the form of "X time" or "X times".
--]]
function format.times( times )
   return n_("%s time", "%s times", times):format(format.number(times))
end


local function _replace(template, index, text)
   local str, unused = string.gsub(template, index, text)
   return str
end
--[[
-- @brief Creates a translatable list of words.
--
-- Taken from https://stackoverflow.com/questions/43081112/localization-of-lists/58033018#58033018
--
--    @tparam table words List of words to translate such as {"one","two","three"}.
--    @treturn string String of the list of words such as "one, two, and three"
--]]
function format.list( words )
   local length = #words
   if length == 1 then return words[1] end
   if length == 2 then
      return _replace(_replace( _("{0} and {1}"), '{0}', words[1]),
         '{1}', words[2])
   end

   local result = _replace( _("{0}, and {1}"), '{1}', words[length])
   while length > 3 do
      length = length - 1
      local mid = _replace( _("{0}, {1}"), '{1}', words[length])
      result = _replace(result, '{0}', mid)
   end
   result = _replace(result, '{0}', words[2])
   result = _replace( _("{0}, {1}"), '{1}', result)
   result = _replace(result, '{0}', words[1])
   return result
end

--[[
   Inspired by https://github.com/hishamhm/f-strings without debug stuff
--]]
local function loads( code, name, env )
   local fn, err = loadstring(code, name)
   if fn then
      setfenv(fn, env)
      return fn
   end
   return nil, err
end
function format.f(str, tab)
   return (str:gsub("%b{}", function(block)
      local code, fmt = block:match("{(.*):(%%.*)}")
      code = code or block:match("{(.*)}")
      local fn, err = loads("return "..code, string.format(_("format expression `%s`"),code), tab)
      if fn then
         return fmt and string.format(fmt, fn()) or tostring(fn())
      else
         error(err, 0)
      end
   end))
end

return format
