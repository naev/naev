notactive = true

function onstealth( p, _po, stealthed )
   if mem.stealthed and not stealthed then
      p:effectAdd( "Ambush Hunter" )
   end
   mem.stealthed = stealthed
end

function init( p, po )
   mem.stealthed = p:flags("stealth")
end

function land(p,po)
   -- At least, ship properties lose the effect bonus.
   -- Still, the icon remains.
   p:effectRm( "Ambush Hunter" )
end

