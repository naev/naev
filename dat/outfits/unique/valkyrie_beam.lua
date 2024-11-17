function init( _p, _po )
   mem.state = false
end

function update( p, _po )
   if mem.state then
      p:effectAdd( "Ride of the Valkyries" )
   end
end

function onshoot( p, _po, on )
   if on then
      p:effectAdd( "Ride of the Valkyries" )
   end
   mem.state = on
   return true -- Doesn't limit anything
end
