function init( _p, _po )
   mem.state = false
end

function update( p, _po )
   if mem.state then
      p:effectAdd( "Ride of the Valkyries" )
   end
end

function onshoot( p, _po )
   p:effectAdd( "Ride of the Valkyries" )
   mem.state = true
   return true -- Doesn't limit anything
end

function ontoggle( _p, _po, on )
   if not on then
      mem.state = false
   end
   return true
end
