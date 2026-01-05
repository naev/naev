local fmt = require "format"

local THRESHOLD = 50

function descextra( _p, _o, _po )
   return fmt.f(_("Can only restore armour when below {threshold}%."),
      {threshold = THRESHOLD})
end

function update( p, po )
   local a = p:health()
   if a > THRESHOLD then
      po:state( "off" )
   else
      po:state( "on" )
   end
end
