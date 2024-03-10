-- Partial reimplementation of https://github.com/hoelzro/lua-term (license: MIT) using Naev markup.

local term = { colours = {} }

function term.colours.bright(s)
   return s  -- Must be composable with a colour, so don't even escape the string.
end

local function makecolour(value)
   local fmt = "#" .. value .. "%s#0"
   local function colourize(s)
      return fmt:format(string.gsub(s, "#", "##"))
   end
   return colourize
end

local colourvalues = {
   red     = "r",
   green   = "g",
   yellow  = "y",
   blue    = "b",
   magenta = "p",
   cyan    = "F", -- sort of
   white   = "w",
}

for c, v in pairs(colourvalues) do
   term.colours[c] = makecolour(v)
end

return term
