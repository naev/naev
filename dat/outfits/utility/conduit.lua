function update( p, po )
   local _a, s = p:health()
   if s == 100 then
      po:state( "on" )
   else
      po:state( "off" )
   end
end
