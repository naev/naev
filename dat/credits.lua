local intro = require "intro"
local lf = require "love.filesystem"
local data = lf.read( "AUTHORS" )

local function lineiter( s )
   if s:sub(-1)~="\n" then s=s.."\n" end
   return s:gmatch("(.-)\n")
end

music.choose("credits")
intro.init()
for l in lineiter(data) do
   intro.text(_(l))
end
intro.run()
