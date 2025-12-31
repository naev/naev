notactive = true

function descextra( _p, _o )
   return "#r".._("Drains fuel on armour damage.").."#0"
end

function onhit( p, _po, armour )
   p:setFuel(math.max(p:fuel()-armour,0))
end
