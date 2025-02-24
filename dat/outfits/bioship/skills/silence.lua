
function onstealth( _p, po, stealthed )
   if stealthed then
      po:state( "on" )
   else
      po:state( "off" )
   end
end

function update( p, po )
   onstealth( p, po, p:flags("stealth") )
end

function init( _p, po )
    --po:clear()
    po:set( "ew_hide", -15 )
end

