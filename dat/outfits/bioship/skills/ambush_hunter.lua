notactive = true

local function _update( p, stealthed )
   if mem.stealthed and not stealthed then
      p:effectAdd( "Ambush Hunter" )
   end
   mem.stealthed = stealthed
end

function init( p, _po )
   mem.stealthed = p:flags("stealth")
end

function onstealth( p, _po, stealthed )
   _update( p, stealthed )
end

function update( p, _po, _dt )
   _update( p, p:flags("stealth") )
end
