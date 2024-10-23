local fmt = require "format"
local naevpedia = require "naevpedia"

local classes = {}

function classes.list( c )
   local out = ""
   local ships = {}
   for k,s in ipairs(ship.getAll()) do
      if s:class()==c and not s:tags().nonaevpedia and s:known() then
         table.insert( ships, s )
      end
   end
   table.sort( ships, function( a, b )
      return a:name() < b:name()
   end )
   for k,s in ipairs(ships) do
      local desc = fmt.f(_("{fct} {class}"), {fct=s:faction(), class=_(s:classDisplay())})
      out = out.."* **["..s:name().."]("..naevpedia.get(s:nameRaw())..")**: "..desc.."\n"
   end
   return out
end

return classes
