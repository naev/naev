function descextra ( _p, _o, _po )
  return "#b".._("Only active when shields are full.").."#0"
end

function update( p, po )
   local s = p:shield()
   if s == 100 then
      po:state( "on" )
   else
      po:state( "off" )
   end
end
