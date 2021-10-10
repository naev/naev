function update( p, po )
   local _a, s = p:health()
   if s < 70 then
      po:state( "off" )
   else
      po:state( "on" )
   end
end
