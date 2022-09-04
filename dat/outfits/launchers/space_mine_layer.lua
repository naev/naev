local spacemine = require "luaspfx.spacemine"

function ontoggle( p, _po, on )
   if not on then return end

   spacemine( p:pos(), p:vel(), p:faction(), {
   } )

   return true
end
