notactive = true

function onstealth( p, _po, stealthed )
   if mem.stealthed and not stealthed then
      p:effectAdd( "Ambush Hunter I" )
   end
   mem.stealthed = stealthed
end

function init( p, _po )
   mem.stealthed = p:flags("stealth")
end
