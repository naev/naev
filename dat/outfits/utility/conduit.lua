function update( p, po )
   local s = p:shield()
   if s == 100 then
      po:state( "on" )
   else
      po:state( "off" )
   end
end
