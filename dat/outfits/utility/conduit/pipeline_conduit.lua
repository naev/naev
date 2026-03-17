require "outfits.utility.conduit.conduit"

-- We want the CPU mod to _always_ be on

function init( p, po )
   mem.on = false
   po:clear()
   update( p, po )
end

function update( p, po )
   local s = p:shield()
   if s == 100 then
      if not mem.on then
         po:state( "on" )
         po:clear()
         mem.on = true
      end
   else
      if mem.on then
         po:state( "off" )
         po:set( "cpu_mod", -5 )
         mem.on = false
      end
   end
end
