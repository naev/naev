--[[--
   Provides string formatting and interpolation facilities.
   The benefits of using it are easy internationalization and consistency with the rest of Naev.
   The number formatters handle plural forms in the user's language, digit separators, and abbreviations.
   String interpolation (format.f) allows for named parameters ("Fly to {planet} in the {system} system"),
   whereas string.format is positional ("Fly to %s in the %s system") and locks translators into the English word order.

   @module format
--]]
local format = {}

--[[--
Converts a nonnegative integer into a human readable string, delimiting every third digit with a comma.
If you pass a more exotic "number" value, you'll get a string back, but you won't attain happiness.

   @param number The number to format. Will be rounded to the nearest integer.
--]]
function format.number( number )
   local numberstring = ""
   if number > -math.huge and number < math.huge then
      number = math.floor(number + 0.5)
      while number >= 1000 do
         numberstring = string.format( ",%03d%s", number % 1000, numberstring )
         number = math.floor(number / 1000)
      end
      numberstring = number % 1000 .. numberstring
   else
      numberstring = tostring(number):gsub("inf", "∞")
   end
   return numberstring
end


--[[--
Converts a number of credits to a string.

   Should be used everywhere a number of credits is displayed.

   @usage tk.msg("", _("You have been paid %s."):format(fmt.credits(credits)))

      @param credits Number of credits.
      @return A string taking the form of "X ¤".
--]]
function format.credits( credits )
   return n_("%s ¤", "%s ¤", credits):format( format.number(credits) )
end


--[[--
Converts an item object or number of credits to reward string ("You have received _").

   @usage vn.na(fmt.reward(money_reward))

      @param reward Thing or number of credits the player is receiving.
                    Avoid passing strings (English or translated) for clarity's sake.
      @return A string taking the form of "You have received X." -- translated and colorized.
--]]
function format.reward( reward )
   local reward_text = (type(reward) == "number") and format.credits(reward) or reward
   return format.f(_("You have received #g{reward}#0."), {reward=reward_text})
end


--[[--
Converts a number of tonnes to a string, using ngettext.

   This adds "tonnes" to the output of fmt.number in a translatable way.
   Should be used everywhere a number of tonnes is displayed.

   @usage tk.msg("", _("You are carrying %s."):format(fmt.tonnes(tonnes)))

   @param tonnes Number of tonnes.
   @return A string taking the form of "X tonne" or "X tonnes".
--]]
function format.tonnes( tonnes )
   return n_("%s tonne", "%s tonnes", tonnes):format( format.number(tonnes) )
end


--[[--
Like fmt.tonnes, but for abbreviations.

   @param tonnes Number of tonnes.
   @return A short string like "22 t" describing the given mass.
--]]
function format.tonnes_short( tonnes )
   -- Translator note: this form represents an abbreviation of "_ tonnes".
   return n_( "%d t", "%d t", tonnes ):format( tonnes )
end


--[[--
Converts a number of jumps to a string, utilizing ngettext.

   This adds "jumps" to the output of fmt.number in a translatable way.
   Should be used everywhere a number of jumps is displayed.

   @usage tk.msg("", _("The system is %s away."):format(fmt.jumps(jumps)))

      @param jumps Number of jumps.
      @return A string taking the form of "X jump" or "X jumps".
--]]
function format.jumps( jumps )
   return n_("%s jump", "%s jumps", jumps):format(format.number(jumps))
end


--[[--
   Converts a number of times (occurrences) to a string, using ngettext.

   This adds "times" to the output of fmt.number in a translatable way.
   Should be used everywhere a number of occurrences is displayed.

   @usage tk.msg("", _("Brush your teeth % per day."):format(fmt.times(times)))

   @param times Number of times.
   @return A string taking the form of "X time" or "X times".
--]]
function format.times( times )
   return n_("%s time", "%s times", times):format(format.number(times))
end

--[[-- A version of string.gsub returning just the string. --]]
local function _replace(template, index, text)
   local str = string.gsub(template, index, text)
   return str
end

--[[--
Creates a translatable list of words.

   Taken from <a href="https://stackoverflow.com/questions/43081112/localization-of-lists/58033018#58033018">this post</a>.

   @tparam table words List of words to translate such as {"one","two","three"}.
   @treturn string String of the list of words such as "one, two, and three"
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

--[[--
String interpolation, inspired by <a href="https://github.com/hishamhm/f-strings">f-strings</a> but closer to Python's str.format().

   Prefer this over string.format because it allows translations to change the word order.
   It also lets you use objects in the formatting (if they support tostring()), whereas string.format can only do this under LuaJIT.

   @usage fmt.f(_("Deliver the loot to {pnt} in the {sys} system"),{pnt=returnpnt, sys=returnsys})
   @usage fmt.f(_("As easy as {1}, {2}, {3}"), {"one", "two", "three"})
   @usage fmt.f(_("A few digits of pi: {1:.2f}"), {math.pi})

   @tparam string str Format string which may include placeholders of the form "{var}" "{var:6.3f}"
                      (where the expression after the colon is any directive string.format understands).
   @tparam table tab Argument table.
--]]
function format.f( str, tab )
   return (str:gsub("%b{}", function(block)
      local key, fmt = block:match("{(.*):(.*)}")
      key = key or block:match("{(.*)}")
      key = tonumber(key) or key  -- Support {1} for printing the first member of the table, etc.
      val = tab[key]
      if val==nil then
         warn(string.format(_("fmt.f: string '%s' has '%s'==nil!"), str, key))
      end
      return fmt and string.format('%'..fmt, val) or tostring(val)
   end))
end

return format
