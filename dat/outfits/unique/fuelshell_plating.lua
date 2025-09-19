notactive = true

function descextra( _p, _o )
   return "#r".._("Drains fuel on armour damage.").."#0"
end

function onhit(p,_po,armour,_shield,_attacker)
   p:setFuel(math.max(p:fuel()-armour,0))
end
