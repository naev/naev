-- Partial reimplementation of https://github.com/hoelzro/lua-term (licence: MIT) using Naev markup.

local term = { colors = {} } -- codespell:ignore colors

function term.colors.bright(s) -- codespell:ignore colors
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
   term.colors[c] = makecolour(v) -- codespell:ignore colors
end

return term
