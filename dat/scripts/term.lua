-- Partial reimplementation of https://github.com/hoelzro/lua-term (license: MIT) using Naev markup.

local term = { colors = {} }

function term.colors.bright(s)
   return s  -- Must be composable with a color, so don't even escape the string.
end

local function makecolor(value)
   local fmt = "#" .. value .. "%s#0"
   local function colorize(s)
      return fmt:format(string.gsub(s, "#", "##"))
   end
   return colorize
end

local colorvalues = {
   red     = "r",
   green   = "g",
   yellow  = "y",
   blue    = "b",
   magenta = "p",
   cyan    = "F", -- sort of
   white   = "w",
}

for c, v in pairs(colorvalues) do
   term.colors[c] = makecolor(v)
end

return term
