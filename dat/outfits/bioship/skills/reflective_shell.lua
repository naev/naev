local fmt = require "format"

notactive = true

local REFLECT = 0.1 -- amount to reflect
local THORN = outfit.get("Bioship Thorn")
local TRIGGER = 50

function descextra()
   local s = THORN:specificstats()
   return fmt.f(_([[Reduces damage taken by {reflect}%. Every {trigger} damage taken, a seeking thorn is launched at the attacker dealing {damage} damage with {penetration}% penetration.]]), {
      reflect  = REFLECT*100,
      damage   = s.damage,
      penetration = s.penetration*100,
      trigger  = TRIGGER,
   })
end

function init( _p, _po )
   mem.dmg = 0
end

function onhit( p, po, armour, _shield, _disable, attacker, pos )
   local dmg = armour
   if dmg <= 0 then
      return
   end

   -- Doesn't actually prevent death!
   p:addHealth( dmg*REFLECT, 0 )

   -- Accumulate damage and break off
   mem.dmg = mem.dmg+dmg
   local n = math.floor(mem.dmg / TRIGGER)
   if n>0 and attacker and attacker:exists() then
      local dir = (attacker:pos()-p:pos()):angle()
      local fan = math.min( 60, n*15 )
      for i=1,n do
         local idir = dir  + ((i-1)/n - 0.5)*fan
         po:munition( p, THORN, attacker, idir, pos )
      end
      mem.dmg = mem.dmg - TRIGGER*n
   end
end
