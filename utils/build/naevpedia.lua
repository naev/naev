local naevpedia = require "naevpedia"
local pot = naevpedia.pot()
local outfile = io.open( arg[1], "w" )
for k,v in ipairs(pot) do
   outfile:write(v)
end
outfile:close()
