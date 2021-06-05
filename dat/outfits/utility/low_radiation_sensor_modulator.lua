function update( p, po )
   if p:flags().stealth then
      po:state( "on" )
   else
      po:state( "off" )
   end
end
