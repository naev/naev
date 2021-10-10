function update( p, po )
   local a = p:health()
   if a > 50 then
      po:state( "off" )
   else
      po:state( "on" )
   end
end
