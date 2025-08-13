
function onstealth( _p, po, stealthed )
   if stealthed then
      po:state( "on" )
   else
      po:state( "off" )
   end
end

function init( _p, po )
   po:set( "ew_hide", -15 )
end
