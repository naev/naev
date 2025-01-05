notactive = true
function onhit(p,_po,armour,_shield,_attacker)
   p:setFuel(math.max(p:fuel()-armour,0))
end
