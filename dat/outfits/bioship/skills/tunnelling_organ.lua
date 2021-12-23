function init( _p, po )
   po:state("on")
end

function jumpin( p, _po )
   p:cooldownCycle()
end
