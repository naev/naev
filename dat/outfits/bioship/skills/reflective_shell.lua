local fmt = require "format"

notactive = true

local REFLECT = 0.1 -- amount to reflect
local THORN = outfit.get("Bioship Thorn")
local TRIGGER = 25

function descextra()
   return fmt.f(_([[Reflect {reflect}% of damage back to attackers. Reflected damage is affected by launcher damage bonus.]]), {
      reflect = REFLECT*100,
   })
end

function init( _p, _po )
   mem.dmg = 0
end

function onhit( p, po, armour, _shield, attacker, pos )
   local dmg = armour
   if dmg <= 0 then
      return
   end

   -- Doesn't actually prevent death!
   p:addHealth( dmg*REFLECT, 0 )

   -- Accumulate damage and break off
   mem.dmg = mem.dmg+dmg
   print( mem.dmg, p:shipstat("absorb") )
   if mem.dmg>TRIGGER and attacker and attacker:exists() then
      --attacker:damage( dmg, nil, 100, nil, p )
      local dir = (attacker:pos()-p:pos()):angle()
      po:munition( p, THORN, attacker, dir, pos )
      mem.dmg = mem.dmg-TRIGGER
   end
end
