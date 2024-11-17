notactive = true

local reflect = 0.05 -- amount to reflect

function onhit( p, _po, armour, _shield, attacker )
   local dmg = armour * reflect * p:shipstat("tur_damage",true)
   p:addHealth( dmg, 0 ) -- doesn't actually prevent death!
   if attacker and attacker:exists() then
      attacker:damage( dmg, nil, 100, nil, p )
   end
end
