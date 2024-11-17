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
      local shipPage = naevpedia.get(s:nameRaw())
      if shipPage then
         local desc = fmt.f(_("{fct} {class}"), {fct=s:faction(), class=_(s:classDisplay())})
         out = out.."* **["..s:name().."]("..shipPage..")**: "..desc.."\n"
      else
         warn( fmt.f(_("Ship {s} has neither naevpedia content nor 'nonaevpedia' tag"), {s=s} ) )
      end
   end
   return out
end

return classes
